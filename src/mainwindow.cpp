/*
    Copyright 2009 Wesley Stessens

    This file is part of QPasm.

    QPasm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QPasm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QPasm. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "highlighter.h"
#include "aboutdialog.h"
#include "pasmthread.h"
#include "listwidgetitem.h"
#include "addmemdialog.h"
#include "linevalidator.h"
#include "configdialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QScrollBar>
#include <QTimer>
#include <QPainter>
#include <QCloseEvent>
#include <QSettings>

#define ADDRESS_ROLE Qt::UserRole
#define VALUE_ROLE   Qt::UserRole + 1

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindowClass) {
    // Setup user interface
    ui->setupUi(this);
    posInfo = new QLabel(this);
    ui->statusBar->addPermanentWidget(posInfo);
    updatePositionInfo();
    ui->splitter->setContextMenuPolicy(Qt::CustomContextMenu);
    QMenu *menu = createPopupMenu();
    menu->setTitle(tr("&View"));
    ui->menuBar->insertMenu(ui->menuBar->actions()[3], menu);
    setUnifiedTitleAndToolBarOnMac(true);
    setWindowTitle(tr("Untitled[*] - QPasm - Pseudo Assembler"));
    defaultLayout = saveState();

    // Some defaults
    changedSinceSave = false;
    changedSinceCompile = true;
    savePath = QDir::currentPath();
    openPath = QDir::currentPath();
    fileName = "";
    timer = NULL;
    nextToInterpret = NULL;
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionPause->setEnabled(false);
    ui->actionStop->setEnabled(false);

    // Initialize PASM library and set up a thread to run assembly programs
    pasm_init();
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<Boolean>("Boolean");
    qRegisterMetaType<ErrorCode>("ErrorCode");
    pasmThread = new PasmThread(this);

    // More initialization
    highlighter = new Highlighter(ui->frameAsmCode->textEdit()->document(), ui->frameAsmCode->textEdit());
    ui->frameAsmCode->setBreakpointList(&bpList);
    updateRegisterInfo();
    enableEditFeatures(false);
    enableMemoryEditFeatures(true);
    updatePalette();
    ui->linePC->setValidator(new LineValidator(this, LineValidator::Unsigned32Bit, LineValidator::Decimal));
    LineValidator *signedValidator = new LineValidator(this, LineValidator::Signed32Bit, LineValidator::Decimal);
    ui->lineRegA->setValidator(signedValidator);
    ui->lineRegB->setValidator(signedValidator);
    setEditButtons();

    // Create a signal bridge between the C callbacks and the Qt signal/slot system
    signalBridge = new QtSignalBridge(this);
    QObject::connect(signalBridge, SIGNAL(registerChanged()),                this, SLOT(updateRegisterInfo()));
    QObject::connect(signalBridge, SIGNAL(memoryChanged(uint32_t, int32_t)), this, SLOT(updateMemoryInfo(uint32_t, int32_t)));
    QObject::connect(signalBridge, SIGNAL(errorMessage(ErrorCode)),          this, SLOT(showErrorMessage(ErrorCode)));
    QObject::connect(signalBridge, SIGNAL(readInputRequested()),             this, SLOT(readInput()));
    QObject::connect(signalBridge, SIGNAL(outputAvailable(int32_t)),         this, SLOT(showOutput(int32_t)));
    QObject::connect(signalBridge, SIGNAL(interpreterReset()),               this, SLOT(reset()));
    QObject::connect(signalBridge, SIGNAL(programStart()),                   this, SLOT(reportProgramStarted()));
    QObject::connect(signalBridge, SIGNAL(programEnd()),                     this, SLOT(reportProgramEnded()));
    QObject::connect(signalBridge, SIGNAL(programPaused(bool)),              this, SLOT(reportProgramPaused(bool)));
    QObject::connect(signalBridge, SIGNAL(nextLine(uint32_t)),               this, SLOT(nextLineToInterpret(uint32_t)));
    QObject::connect(signalBridge, SIGNAL(breakpointSet(uint32_t, bool)),    this, SLOT(setBreakpoint(uint32_t, bool)));
    QObject::connect(signalBridge, SIGNAL(memoryCellDeleted(uint32_t)),      this, SLOT(deleteMemoryCell(uint32_t)));
    QObject::connect(signalBridge, SIGNAL(rebootMePlz()),                    this, SLOT(on_actionRun_triggered()));

    // Connect other signals
    QObject::connect(ui->frameAsmCode->textEdit(), SIGNAL(cursorPositionChanged()), this, SLOT(updatePositionInfo()));
    QObject::connect(ui->frameAsmCode->textEdit()->document(), SIGNAL(modificationChanged(bool)), this, SLOT(asmCodeModifiedChanged(bool)));
    QObject::connect(ui->frameAsmCode->textEdit()->document(), SIGNAL(contentsChanged()), this, SLOT(asmCodeTextChanged()));
    QObject::connect(ui->frameAsmCode->textEdit(), SIGNAL(undoAvailable(bool)), this, SLOT(enableUndoButton(bool)));
    QObject::connect(ui->frameAsmCode->textEdit(), SIGNAL(redoAvailable(bool)), this, SLOT(enableRedoButton(bool)));
    QObject::connect(ui->frameAsmCode->textEdit(), SIGNAL(selectionChanged()), this, SLOT(setEditButtons()));
    QObject::connect(ui->frameAsmCode, SIGNAL(lineClicked(int)), this, SLOT(toggleBreakpoint(int)));
    QObject::connect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));

    // Load settings
    loadSettings();
}

MainWindow::~MainWindow() {
    saveLayout();
    pasm_free();
    pasmThread->terminate();
    delete ui;
}

void MainWindow::loadSettings() {
    QSettings settings("config.ini", QSettings::IniFormat, this);
    restoreState(settings.value("state", QByteArray()).toByteArray());
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
    timedStepInterval = settings.value("timedStepInterval", 500).toInt();
    QFont f;
    f.setStyleHint(QFont::TypeWriter);
    f.setFamily(settings.value("font", "Monospace").toString());
    ui->frameAsmCode->setNewFont(f);
    updateColorScheme();
}

void MainWindow::restoreDefaultLayout() {
    restoreState(defaultLayout);
}

void MainWindow::saveLayout() {
    QSettings settings("config.ini", QSettings::IniFormat, this);
    settings.setValue("state", saveState());
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::setEditorFont(const QFont &font) {
    ui->frameAsmCode->setNewFont(font);
    QSettings settings("config.ini", QSettings::IniFormat, this);
    settings.setValue("font", font);
}

void MainWindow::updateColorScheme() {
    QSettings settings("config.ini", QSettings::IniFormat, this);
    QString scheme = settings.value("colorScheme", "automatic").toString();
    if (scheme == "custom") {
        QList<QVariant> defaultColors;
        defaultColors << QColor(Qt::darkBlue) << QColor(Qt::darkGray) << QColor(Qt::darkRed) << QColor(Qt::darkYellow)
                      << QColor(Qt::darkMagenta) << QColor(Qt::darkGreen) << QColor(Qt::darkCyan) << QColor(Qt::blue);
        QList<QVariant> colors = settings.value("customColors", QVariant(defaultColors)).toList();
        if (colors.size() < 8)
            colors = defaultColors;
        highlighter->setCustomColors(colors);
    }
    QObject::disconnect(ui->frameAsmCode->textEdit()->document(), SIGNAL(contentsChanged()), this, SLOT(asmCodeTextChanged()));
    highlighter->setColorScheme(scheme);
    QObject::connect(ui->frameAsmCode->textEdit()->document(), SIGNAL(contentsChanged()), this, SLOT(asmCodeTextChanged()));
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::PaletteChange)
        updatePalette();
}

void MainWindow::updatePalette() {
    QColor textColor = palette().text().color();
    QString color = palette().window().color().value() < 128 ? (textColor.value() >= 128 ? textColor.name() : "white") :
                                                               (textColor.value() < 128 ? textColor.name() : "black");
    ui->flagZero->setStyleSheet("color: " + color + ";");
    ui->flagNeg->setStyleSheet("color: " + color + ";");
    ui->flagOverflow->setStyleSheet("color: " + color + ";");
    if (!ui->linePC->styleSheet().isEmpty()) {
        QString lineStyle = "border: none; background: transparent; color: " + color + ";";
        ui->linePC->setStyleSheet(lineStyle);
        ui->lineRegA->setStyleSheet(lineStyle);
        ui->lineRegB->setStyleSheet(lineStyle);
    }
    QObject::disconnect(ui->frameAsmCode->textEdit()->document(), SIGNAL(contentsChanged()), this, SLOT(asmCodeTextChanged()));
    highlighter->rehighlight();
    QObject::connect(ui->frameAsmCode->textEdit()->document(), SIGNAL(contentsChanged()), this, SLOT(asmCodeTextChanged()));
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (handleUnsavedDocument())
        event->ignore();
    else
        event->accept();
}

void MainWindow::enableMemoryEditFeatures(bool enable) {
    ui->memAddBtn->setEnabled(enable);
    ui->memDelBtn->setEnabled(enable);
    ui->memClearBtn->setEnabled(enable);
    if (enable)
        ui->tableMem->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    else
        ui->tableMem->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::enableEditFeatures(bool enable) {
    ui->linePC->setReadOnly(!enable);
    ui->lineRegA->setReadOnly(!enable);
    ui->lineRegB->setReadOnly(!enable);
    ui->flagZero->setEnabled(enable);
    ui->flagNeg->setEnabled(enable);
    ui->flagOverflow->setEnabled(enable);
    if (enable) {
        ui->linePC->setStyleSheet("");
        ui->lineRegA->setStyleSheet("");
        ui->lineRegB->setStyleSheet("");
    } else {
        Register reg = pasm_getRegister();
        QString lineStyle = "border: none; background: transparent;";
        ui->linePC->setStyleSheet(lineStyle);
        updatePalette();
    }
    enableMemoryEditFeatures(enable);
}

void MainWindow::toggleBreakpoint(int line) {
    if (bpList.contains(line))
        pasm_setBreakpoint(line, _FALSE);
    else
        pasm_setBreakpoint(line, _TRUE);
}

void MainWindow::updateRegisterInfo() {
    const Register reg = pasm_getRegister();

    ui->linePC->setText(QString::number(reg.programCounter));
    ui->lineRegA->setText(QString::number(reg.a));
    ui->lineRegB->setText(QString::number(reg.b));

    ui->flagNeg->setChecked(reg.negativeFlag);
    ui->flagZero->setChecked(reg.zeroFlag);
    ui->flagOverflow->setChecked(reg.overflowFlag);
}

void MainWindow::updateMemoryInfo(uint32_t address, int32_t value) {
    QTableWidgetItem *addressItem, *valueDecItem, *valueHexItem, *valueBinItem, *instructionItem;
    int rowNum;
    char *cmd;

    QObject::disconnect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
    if (memoryTableMap.contains(address)) { // This table widget item already exists
        addressItem = memoryTableMap[address];
        rowNum = addressItem->row();
        valueDecItem = ui->tableMem->item(rowNum, 1);
        valueHexItem = ui->tableMem->item(rowNum, 2);
        valueBinItem = ui->tableMem->item(rowNum, 3);
        instructionItem = ui->tableMem->item(rowNum, 4);
    } else { // Create a new table widget item
        addressItem = new QTableWidgetItem;
        valueDecItem = new QTableWidgetItem;
        valueHexItem = new QTableWidgetItem;
        valueBinItem = new QTableWidgetItem;
        instructionItem = new QTableWidgetItem;
        rowNum = ui->tableMem->rowCount();
        ui->tableMem->insertRow(rowNum);
        ui->tableMem->setItem(rowNum, 0, addressItem);
        ui->tableMem->setItem(rowNum, 1, valueDecItem);
        ui->tableMem->setItem(rowNum, 2, valueHexItem);
        ui->tableMem->setItem(rowNum, 3, valueBinItem);
        ui->tableMem->setItem(rowNum, 4, instructionItem);
        addressItem->setData(Qt::DisplayRole, address);
        addressItem->setData(ADDRESS_ROLE, address);
        addressItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        memoryTableMap[address] = addressItem;
        ui->tableMem->sortItems(0, Qt::AscendingOrder); // Sort on address (ascending)
    }

    addressItem->setData(VALUE_ROLE, value);
    addressItem->setIcon(QIcon(pasm_isDynamicMemory(address) ? ":/res/img/dynamic-memory.png" : ":/res/img/static-memory.png"));
    valueDecItem->setData(Qt::DisplayRole, value);
    valueHexItem->setData(Qt::DisplayRole, QString("%1").arg(QString::number((unsigned int)value, 16), 8, '0'));
    valueBinItem->setData(Qt::DisplayRole, QString("%1").arg(QString::number((unsigned int)value, 2), 32, '0'));

    // If this value were an instruction, what instruction would it be (NOP if invalid instruction)
    cmd = pasm_instructionToString(pasm_valueToInstruction(value));
    instructionItem->setData(Qt::DisplayRole, QString(cmd));
    free(cmd);

    updateCurrentLineMarker();
    QObject::connect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
    ui->tableMem->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::deleteMemoryCell(uint32_t address) {
    if (memoryTableMap.contains(address)) {
        ui->tableMem->removeRow(memoryTableMap[address]->row());
        memoryTableMap.remove(address);
    }
}

void MainWindow::showErrorMessage(ErrorCode error) {
    QString errorMessage;
    Error *errorInfo = NULL;
    bool cancel = false;

    switch (error) {
        case HOST_MEMORY_EXHAUSTED:
            errorMessage = tr("There's not enough memory available on your physical machine to perform the necessary operations.");
            break;
        case UNSUPPORTED_OPCODE:
            errorMessage = tr("The assembler program contains an unsupported opcode.");
            break;
        case UNSUPPORTED_ADDRESSMODE:
            errorMessage = tr("The assembler program contains an unsupported address mode.");
            break;
        case UNSUPPORTED_CALLBACK:
            errorMessage = tr("Frontend tried to register an unsupported callback type.");
            break;
        case OPERAND_REQUIRED:
            errorMessage = tr("An operand was required for one of the instructions but was not given.");
            break;
        case INVALID_MEMORY_REGION:
            errorInfo = pasm_popError();
            errorMessage = tr("The assembler program tried to access an invalid memory region (memory address %1)").arg(
                              errorInfo ? QString::number(errorInfo->address) : "?");
            if (errorInfo)
                pasm_freeError(errorInfo);
            break;
        case CALLSTACK_UNDERFLOW:
            errorMessage = tr("Call stack underflow. There's no address to return to.");
            break;
        case DIVISION_BY_ZERO:
            errorInfo = pasm_popError();
            errorMessage = tr("Division by zero cannot be done. Assembler program tried to divide by zero during execution of instruction at address %1.").arg(
                              errorInfo ? QString::number(errorInfo->address) : "?");
            if (errorInfo)
                pasm_freeError(errorInfo);
            break;
        case UNRESOLVABLE_SYMBOL:
            errorInfo = pasm_popError();
            errorMessage = tr("The symbol <strong>%1</strong> which was encountered at line %2 couldn't be resolved.").arg(
                              errorInfo && errorInfo->symbol ? QString(errorInfo->symbol) : "?", errorInfo ? QString::number(errorInfo->address) : "?");
            if (errorInfo)
                pasm_freeError(errorInfo);
            break;
        case OVERFLOW:
            errorInfo = pasm_popError();
            errorMessage = tr("<strong>Warning:</strong> overflow caused by instruction at address %1.").arg(errorInfo ? QString::number(errorInfo->address) : "?");
            ui->textOutput->append(errorMessage);
            if (errorInfo)
                pasm_freeError(errorInfo);
            // No break, because this is also no real error
        case NO_ERROR:
            cancel = true;
            break;
        default:
            errorMessage = tr("Unknown error");
    }

    if (!cancel)
        QMessageBox::critical(this, tr("Error occured"),
                              "<strong>" + tr("An error has occured:") + "</strong><br />" + errorMessage);
}

void MainWindow::readInput() {
    int input = QInputDialog::getInt(this, tr("Input required"),
                                     tr("The assembler program requests input from the user:"),
                                     0, MIN_SIGNED_NEGATIVE_32BIT_NUMBER, MAX_SIGNED_POSITIVE_32BIT_NUMBER);
    pasm_setInput(input);
}

void MainWindow::showOutput(int32_t value) {
    ui->textOutput->append(tr("Output:") + " <strong>" + QString::number(value) + "</strong>");
}

void MainWindow::reset() {
    for (int i = ui->tableMem->rowCount() - 1; i >= 0; --i)
        ui->tableMem->removeRow(i);
    memoryTableMap.clear();
    nextToInterpret = NULL;
    updateRegisterInfo();
}

void MainWindow::reportProgramStarted() {
    ui->textOutput->setHtml("<em>" + tr("Program started.") + "</em>");
    ui->frameAsmCode->setRunning(true);
    ui->actionStop->setEnabled(true);
}

void MainWindow::reportProgramEnded() {
    ui->actionCompile->setEnabled(true);
    ui->actionStep->setEnabled(true);
    ui->actionTimedStep->setEnabled(true);
    ui->actionRun->setEnabled(true);
    ui->actionPause->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->textOutput->append("<em>" + tr("Program ended.") + "</em><br />");
    if (timer) {
        timer->stop();
        timer->deleteLater();
        timer = NULL;
    }
    nextLineToInterpret(0);
    ui->frameAsmCode->setCurrentLine(-1);
    ui->frameAsmCode->setRunning(false);
    enableEditFeatures(false);
    enableMemoryEditFeatures(true);
}

void MainWindow::reportProgramPaused(bool paused) {
    if (paused) {
        ui->actionCompile->setEnabled(true);
        ui->actionStep->setEnabled(true);
        ui->actionTimedStep->setEnabled(true);
        ui->actionRun->setEnabled(true);
        ui->actionPause->setEnabled(false);
        if (timer) {
            timer->stop();
            timer->deleteLater();
            timer = NULL;
        }
        enableEditFeatures(true);
    }
}

void MainWindow::updatePositionInfo() {
    posInfo->setText(tr("line") + " " + QString::number(ui->frameAsmCode->textEdit()->textCursor().blockNumber()) + ", " +
                     tr("column") + " " + QString::number(ui->frameAsmCode->textEdit()->textCursor().columnNumber()));
}

void MainWindow::updateCurrentLineMarker() {
    if (nextToInterpret) {
        QPixmap pm(pasm_isDynamicMemory(nextToInterpret->data(ADDRESS_ROLE).toUInt()) ?
                   ":/res/img/dynamic-memory.png" : ":/res/img/static-memory.png");
        QPainter p(&pm);
        p.setBrush(Qt::red);
        p.setPen(Qt::NoPen);
        p.translate(pm.rect().center() - pm.rect().center() / 2.5);
        p.scale(0.4, 0.4);
        p.drawChord(pm.rect(), 0, 5760 /* 360 * 16 */);
        nextToInterpret->setIcon(QIcon(pm));
    }
}

void MainWindow::nextLineToInterpret(uint32_t address) {
    QTableWidgetItem *addressItem, *valueDecItem, *valueHexItem, *valueBinItem, *instructionItem;
    QFont font;
    int rowNum;

    QObject::disconnect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
    if (memoryTableMap.contains(address)) {
        if (nextToInterpret) {
            rowNum = nextToInterpret->row();
            valueDecItem = ui->tableMem->item(rowNum, 1);
            valueHexItem = ui->tableMem->item(rowNum, 2);
            valueBinItem = ui->tableMem->item(rowNum, 3);
            instructionItem = ui->tableMem->item(rowNum, 4);
            font.setBold(false);
            nextToInterpret->setIcon(QIcon(pasm_isDynamicMemory(address) ? ":/res/img/dynamic-memory.png" : ":/res/img/static-memory.png"));
            nextToInterpret->setFont(font);
            valueDecItem->setFont(font);
            valueHexItem->setFont(font);
            valueBinItem->setFont(font);
            instructionItem->setFont(font);
        }
        rowNum = memoryTableMap[address]->row();
        addressItem = memoryTableMap[address];
        valueDecItem = ui->tableMem->item(rowNum, 1);
        valueHexItem = ui->tableMem->item(rowNum, 2);
        valueBinItem = ui->tableMem->item(rowNum, 3);
        instructionItem = ui->tableMem->item(rowNum, 4);
        font = QFont(addressItem->font());
        font.setBold(true);
        nextToInterpret = addressItem;
        updateCurrentLineMarker();
        addressItem->setFont(font);
        valueDecItem->setFont(font);
        valueHexItem->setFont(font);
        valueBinItem->setFont(font);
        instructionItem->setFont(font);
        ui->tableMem->scrollToItem(addressItem, QAbstractItemView::PositionAtCenter);
    }
    ui->frameAsmCode->setCurrentLine(address);
    QObject::connect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
}

bool MainWindow::saveFile(const QString &fileName) {
    QString saveAs = fileName;
    bool cancel = false;

    if (saveAs == NULL) {
        QDir directory;
        QFileDialog saveDialog(this);
        saveDialog.setDefaultSuffix("asm");
        saveDialog.setFileMode(QFileDialog::AnyFile);
        saveDialog.setWindowTitle(tr("Save as..."));
        saveDialog.setNameFilter(tr("Assembler programs (*.asm);;All files (*)"));
        saveDialog.setDirectory(savePath);
        saveDialog.setAcceptMode(QFileDialog::AcceptSave);
        if (saveDialog.exec() == QDialog::Accepted) {
            saveAs = saveDialog.selectedFiles().first();
            savePath = directory.filePath(saveAs);
        } else
            cancel = true;
    }

    if (!cancel) {
        QFile file(saveAs);
        if (file.open(QFile::WriteOnly | QFile::Truncate)) {
            QTextStream out(&file);
            out << ui->frameAsmCode->textEdit()->toPlainText() << "\n";
            this->fileName = saveAs;
            QFileInfo fileInfo(this->fileName);
            setWindowTitle(fileInfo.fileName() + "[*] - QPasm - Pseudo Assembler");
            setWindowModified(false);
            ui->frameAsmCode->textEdit()->document()->setModified(false);
            changedSinceSave = false;
        } else {
            QMessageBox::critical(this, tr("Could not save file"), tr("Could not save the file. The file couldn't be opened or created. Please check your permissions."),
                                  QMessageBox::Ok);
            cancel = true;
        }
    }

    return cancel;
}

bool MainWindow::handleUnsavedDocument() {
    QMessageBox::StandardButton answer;
    bool cancel = false;

    if (changedSinceSave)
        answer = QMessageBox::question(this, tr("Unsaved document"), tr("The current document has not been saved yet."),
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    else
        answer = QMessageBox::Discard;

    if (answer == QMessageBox::Save)
        cancel = saveFile(NULL);
    else if (answer == QMessageBox::Cancel)
        cancel = true;
    return cancel;
}

bool MainWindow::handleCodeMemSync() {
    QMessageBox::StandardButton answer;
    bool cancel = false;

    if (changedSinceCompile)
        answer = QMessageBox::question(this, tr("Code has changed after last compile"), tr("The code has changed after the last compile. Do you want to recompile before running?"),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
    else
        answer = QMessageBox::Discard;

    if (answer == QMessageBox::Yes)
        on_actionCompile_triggered();
    else if (answer == QMessageBox::Cancel)
        cancel = true;
    return cancel;
}

void MainWindow::on_actionCompile_triggered() {
    QByteArray ba = ui->frameAsmCode->textEdit()->toPlainText().toLatin1();
    pasm_compileProgram(ba.data(), ui->frameAsmCode->textEdit()->document()->lineCount());
    changedSinceCompile = false;
    nextLineToInterpret(0);
}

void MainWindow::on_actionStep_triggered() {
    if (timer)
        timer->stop(); // Stop step timer
    else
        enableEditFeatures(true);
    if (!changedSinceCompile || (changedSinceCompile && !handleCodeMemSync()))
        pasm_step();
    if (timer)
        timer->start(); // Restart step timer
}

void MainWindow::setTimedStepInterval(int msec) {
    if (timer)
        timer->setInterval(msec);
    QSettings settings("config.ini", QSettings::IniFormat, this);
    settings.setValue("timedStepInterval", msec);
}

void MainWindow::on_actionTimedStep_triggered() {
    if (!timer)
        timer = new QTimer(this);
    ui->actionCompile->setEnabled(false);
    ui->actionStep->setEnabled(false);
    ui->actionTimedStep->setEnabled(false);
    ui->actionRun->setEnabled(true);
    ui->actionPause->setEnabled(true);
    enableEditFeatures(false);
    timer->setInterval(timedStepInterval);
    disconnect(timer, 0, 0, 0);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_actionStep_triggered()));
    if (timer->isActive())
        timer->stop();
    timer->start();
    on_actionStep_triggered();
}

void MainWindow::on_actionPause_triggered() {
    if (timer) {
        timer->stop();
        timer->deleteLater();
        timer = NULL;
    }
    pasm_pauseProgram();
}

void MainWindow::on_actionStop_triggered() {
    if (timer) {
        timer->stop();
        timer->deleteLater();
        timer = NULL;
    }
    pasm_stopProgram();
}

void MainWindow::on_actionRun_triggered() {
    if (timer) {
        timer->stop();
        timer->deleteLater();
        timer = NULL;
    }
    if (!changedSinceCompile || (changedSinceCompile && !handleCodeMemSync())) {
        ui->actionCompile->setEnabled(false);
        ui->actionStep->setEnabled(false);
        ui->actionTimedStep->setEnabled(false);
        ui->actionRun->setEnabled(false);
        ui->actionPause->setEnabled(true);
        enableEditFeatures(false);
        pasmThread->start();
    }
}

void MainWindow::on_actionExit_triggered() {
    qApp->quit();
}

void MainWindow::on_flagZero_clicked() {
    Register reg = pasm_getRegister();
    reg.zeroFlag = static_cast<Boolean>(ui->flagZero->isChecked());
    pasm_setRegister(reg);
    updateRegisterInfo();
}

void MainWindow::on_flagNeg_clicked() {
    Register reg = pasm_getRegister();
    reg.negativeFlag = static_cast<Boolean>(ui->flagNeg->isChecked());
    pasm_setRegister(reg);
    updateRegisterInfo();
}

void MainWindow::on_flagOverflow_clicked() {
    Register reg = pasm_getRegister();
    reg.overflowFlag = static_cast<Boolean>(ui->flagOverflow->isChecked());
    pasm_setRegister(reg);
    updateRegisterInfo();
}

void MainWindow::on_actionNew_triggered() {
    if (!handleUnsavedDocument()) {
        pasm_reset();
        ui->textOutput->clear();
        ui->frameAsmCode->textEdit()->clear();
        fileName = "";
        setWindowTitle(tr("Untitled[*] - QPasm - Pseudo Assembler"));
        setWindowModified(false);
        ui->frameAsmCode->textEdit()->document()->setModified(false);
        changedSinceSave = false;
        changedSinceCompile = true;
        pasm_reset();
    }
}

void MainWindow::asmCodeTextChanged() {
    changedSinceCompile = true;
}

void MainWindow::asmCodeModifiedChanged(bool changed) {
    changedSinceSave = changed;
    setWindowModified(changed);
}

void MainWindow::enableUndoButton(bool enable) {
    ui->actionUndo->setEnabled(enable);
}

void MainWindow::enableRedoButton(bool enable) {
    ui->actionRedo->setEnabled(enable);
}

void MainWindow::setEditButtons() {
    ui->actionCut->setEnabled(ui->frameAsmCode->textEdit()->textCursor().hasSelection());
    ui->actionCopy->setEnabled(ui->frameAsmCode->textEdit()->textCursor().hasSelection());
}

void MainWindow::setBreakpoint(uint32_t address, bool enabled) {
    if (enabled && !bpList.contains(address)) {
        bpList << static_cast<int>(address);
        ListWidgetItem *item = new ListWidgetItem(ui->bpList);
        item->setData(Qt::DisplayRole, address);
        item->setData(ADDRESS_ROLE, address);
        item->setText(QString("Address ") + QString::number(address));
        ui->bpList->addItem(item);
        ui->bpList->sortItems(Qt::AscendingOrder);
        ui->frameAsmCode->update();
    } else if (!enabled) {
        bpList.removeAll(address);
        for (int i = 0; i < ui->bpList->count(); ++i) {
            if (ui->bpList->item(i)->data(ADDRESS_ROLE) == address) {
                delete ui->bpList->takeItem(i);
                ui->frameAsmCode->update();
            }
        }
    }
}

void MainWindow::on_actionSave_triggered() {
    if (fileName.isEmpty())
        saveFile(NULL);
    else
        saveFile(fileName);
}

void MainWindow::on_actionSaveAs_triggered() {
    saveFile(NULL);
}

void MainWindow::on_actionOpen_triggered() {
    QDir directory;
    QFileDialog openDialog(this);
    QString fileName;
    bool cancel = handleUnsavedDocument();

    if (!cancel) {
        openDialog.setDefaultSuffix("asm");
        openDialog.setFileMode(QFileDialog::AnyFile);
        openDialog.setWindowTitle(tr("Choose the assembler program that you want to open..."));
        openDialog.setNameFilter(tr("Assembler programs") + " (*.asm);;" + tr("All files") + " (*)");
        openDialog.setDirectory(openPath);
        openDialog.setAcceptMode(QFileDialog::AcceptOpen);
        if (openDialog.exec() == QDialog::Accepted) {
            fileName = openDialog.selectedFiles().first();
            openPath = directory.filePath(fileName);
        } else
            cancel = true;
    }

    if (!cancel) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&file);
            ui->frameAsmCode->textEdit()->setUndoRedoEnabled(false);
            ui->frameAsmCode->textEdit()->clear();
            while (!in.atEnd())
                ui->frameAsmCode->textEdit()->append(in.readLine());
            QTextCursor c = ui->frameAsmCode->textEdit()->textCursor();
            c.setPosition(0);
            ui->frameAsmCode->textEdit()->setTextCursor(c);
            ui->frameAsmCode->textEdit()->verticalScrollBar()->setValue(0);
            ui->frameAsmCode->textEdit()->setUndoRedoEnabled(true);
            pasm_reset();
            ui->textOutput->clear();
            this->fileName = fileName;
            QFileInfo fileInfo(this->fileName);
            setWindowTitle(fileInfo.fileName() + "[*] - " + tr("QPasm - Pseudo Assembler"));
            setWindowModified(false);
            ui->frameAsmCode->textEdit()->document()->setModified(false);
            changedSinceSave = false;
            changedSinceCompile = true;
            pasm_reset();
        } else {
            QMessageBox::critical(this, tr("Could not open file"), tr("Could not open the file. The file couldn't be opened. Please check your permissions."),
                                  QMessageBox::Ok);
        }
    }
}

void MainWindow::on_actionAboutQPasm_triggered() {
    AboutDialog aboutDialog;
    aboutDialog.exec();
}

void MainWindow::on_actionUndo_triggered() {
    ui->frameAsmCode->textEdit()->undo();
}

void MainWindow::on_actionRedo_triggered() {
    ui->frameAsmCode->textEdit()->redo();
}

void MainWindow::on_actionCut_triggered() {
    ui->frameAsmCode->textEdit()->cut();
}

void MainWindow::on_actionCopy_triggered() {
    ui->frameAsmCode->textEdit()->copy();
}

void MainWindow::on_actionPaste_triggered() {
    ui->frameAsmCode->textEdit()->paste();
}

void MainWindow::on_bpAddBtn_clicked() {
    pasm_setBreakpoint(ui->frameAsmCode->textEdit()->textCursor().blockNumber(), _TRUE);
}

void MainWindow::on_bpDelBtn_clicked() {
    if (ui->bpList->currentItem())
        pasm_setBreakpoint(ui->bpList->currentItem()->data(ADDRESS_ROLE).toUInt(), _FALSE);
}

void MainWindow::on_bpClearBtn_clicked() {
    QList<int> allBp(bpList);
    for (int i = 0; i < allBp.size(); ++i)
        pasm_setBreakpoint(allBp.at(i), _FALSE);
}

void MainWindow::on_checkEnableBp_toggled(bool checked) {
    pasm_enableDebugger(static_cast<Boolean>(checked));
    ui->frameAsmCode->setBreakpointsEnabled(checked);
}

void MainWindow::on_linePC_textEdited(const QString &text) {
    Register reg = pasm_getRegister();
    reg.programCounter = text.toUInt();
    pasm_setRegister(reg);
}

void MainWindow::on_lineRegA_textEdited(const QString &text) {
    Register reg = pasm_getRegister();
    reg.a = text.toUInt();
    pasm_setRegister(reg);
}

void MainWindow::on_lineRegB_textEdited(const QString &text) {
    Register reg = pasm_getRegister();
    reg.b = text.toUInt();
    pasm_setRegister(reg);
}

void MainWindow::memItemChanged(QTableWidgetItem* item)
{
    int32_t origValue = ui->tableMem->item(item->row(), 0)->data(VALUE_ROLE).toInt();
    int32_t value;
    uint32_t origAddress = ui->tableMem->item(item->row(), 0)->data(ADDRESS_ROLE).toUInt();
    uint32_t address;
    bool ok;
    Instruction instruction;
    ErrorCode error;
    char *cmd = NULL;
    char *unresolvedLabel = NULL;
    QByteArray ba;

    switch (item->column()) {
    case 0: // Address
        address = item->text().toUInt();
        QObject::disconnect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
        item->setData(Qt::DisplayRole, origAddress);
        QObject::connect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
        if (pasm_memoryCellExists(address)) {
            if (QMessageBox::question(this, tr("Switch values of memory addresses?"),
                                      tr("The given memory address already exists. Would you like to switch the contents of both memory addresses?"),
                                      QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Yes) == QMessageBox::Yes) {
                int32_t tmpValue = pasm_readValue(address);
                pasm_writeValue(address, origValue);
                pasm_writeValue(origAddress, tmpValue);
            }
        } else {
            pasm_writeValue(address, origValue);
            if (!pasm_deleteMemoryCell(origAddress)) {
                QMessageBox::information(this, tr("Cannot delete static memory cell"),
                                         tr("You copied the contents of a static memory cell to a new dynamic memory cell. Since static memory "
                                            "can't be deleted the static memory cell will be initialized with the value 0."), QMessageBox::Ok, QMessageBox::Ok);
                pasm_writeValue(origAddress, 0);
            }
        }
        break;
    case 1: // Decimal value
        value = item->text().toInt();
        QObject::disconnect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
        item->setData(Qt::DisplayRole, origValue);
        QObject::connect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
        pasm_writeValue(origAddress, value);
        break;
    case 2: // Hexadecimal value
        value = item->text().toInt(&ok, 16);
        QObject::disconnect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
        item->setData(Qt::DisplayRole, QString("%1").arg(QString::number((unsigned int)origValue, 16), 8, '0'));
        QObject::connect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
        if (ok)
            pasm_writeValue(origAddress, value);
        break;
    case 3: // Binary value
        value = item->text().toInt(&ok, 2);
        QObject::disconnect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
        item->setData(Qt::DisplayRole, QString("%1").arg(QString::number((unsigned int)origValue, 2), 32, '0'));
        QObject::connect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
        if (ok)
            pasm_writeValue(origAddress, value);
        break;
    case 4: // Instruction
        ba = item->text().toLatin1();
        instruction = pasm_stringToInstruction(ba.data(), &error, NULL, &unresolvedLabel);
        cmd = pasm_instructionToString(pasm_valueToInstruction(origValue));
        if (cmd) {
            QObject::disconnect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
            item->setData(Qt::DisplayRole, QString(cmd));
            QObject::connect(ui->tableMem, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(memItemChanged(QTableWidgetItem*)));
            free(cmd);
        }
        if (error == NO_ERROR) {
            value = pasm_instructionToValue(instruction);
            pasm_writeValue(origAddress, value);
            if (unresolvedLabel) { /* The new instruction refers to another address by using a label */
                PendingItem *item = pasm_addPendingItem(origAddress, unresolvedLabel);
                pasm_resolveItem(item, NULL);
                pasm_freeSymbolData(_FALSE);
            }
        }
        break;
    default:
        break;
    }
}

void MainWindow::on_memAddBtn_clicked() {
    AddMemDialog dialog(this);
    dialog.exec();
}

void MainWindow::on_memDelBtn_clicked() {
    if (ui->tableMem->item(ui->tableMem->currentRow(), 0)) {
        uint32_t address = ui->tableMem->item(ui->tableMem->currentRow(), 0)->data(ADDRESS_ROLE).toUInt();
        QString msg;
        if (pasm_isDynamicMemory(address))
            msg = tr("Are you sure you want to delete the selected dynamic memory cell?");
        else
            msg = tr("Can't delete static memory cells. Do you want to set the selected static memory cell to 0?");

        if (QMessageBox::question(this, tr("Delete memory cell?"), msg, QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Yes) {
            if (!pasm_deleteMemoryCell(address))
                pasm_writeValue(address, 0);
        }
    }
}

void MainWindow::on_memClearBtn_clicked() {
    if (QMessageBox::question(this, tr("Clear all memory?"), tr("Do you want to delete all dynamic memory allocated and set the contents of all static memory to 0 for this assembler program?"),
                              QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Yes) {
        QHash<uint32_t, QTableWidgetItem*> memoryMapCopy(memoryTableMap);
        for (int i = 0; i < memoryMapCopy.keys().count(); ++i) {
            if (!pasm_deleteMemoryCell(memoryMapCopy.keys().at(i)))
                pasm_writeValue(memoryMapCopy.keys().at(i), 0);
        }
    }
}

void MainWindow::setSplitterOrientationHorizontal() {
    ui->splitter->setOrientation(Qt::Horizontal);
}

void MainWindow::setSplitterOrientationVertical() {
    ui->splitter->setOrientation(Qt::Vertical);
}

void MainWindow::on_splitter_customContextMenuRequested(QPoint pos) {
    Q_UNUSED(pos);
    QMenu menu(ui->splitter);
    menu.addAction(tr("Layout horizontally"), this, SLOT(setSplitterOrientationHorizontal()));
    menu.addAction(tr("Layout vertically"), this, SLOT(setSplitterOrientationVertical()));
    menu.exec(QCursor::pos() + QPoint(1, 1));
}

void MainWindow::on_actionSettings_triggered() {
    ConfigDialog dialog(this);
    dialog.exec();
}

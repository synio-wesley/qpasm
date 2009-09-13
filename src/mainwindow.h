/*
    Copyright 2009 Wesley Stessens

    This file is part of QPasm.

    QPasm is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QPasm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with QPasm. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/**
  * \mainpage QPasm - Pseudo Assembler
  * QPasm makes use of the PASM Library to provide a small Integrated Development Environment for small pseudo-assembler programs.<br /><br />
  * QPasm was developed by Wesley Stessens in 2009 in assignment of Hasselt University.
  */

#include "libpasm/pasm.h"
#include "qtsignalbridge.h"
#include <QMainWindow>
#include <QTableWidgetItem>
#include <stdint.h>

class Highlighter;
class QTimer;
class PasmThread;
class QLabel;
class QTranslator;

namespace Ui {
    class MainWindowClass;
}

/**
  * Main window of QPasm.
  */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
      * Constructor.
      * \param parent Parent object
      */
    MainWindow(QWidget *parent = 0);
    void restoreDefaultLayout();
    void setTimedStepInterval(int msec);
    void setEditorFont(const QFont &font);
    void updateColorScheme();

    /**
      * Destructor.
      */
    ~MainWindow();

private:
    void loadSettings();
    void saveLayout();
    bool saveFile(const QString &fileName);
    bool handleUnsavedDocument();
    bool handleCodeMemSync();
    Ui::MainWindowClass *ui; ///< Elements in the user interface are accessible through this
    QtSignalBridge *signalBridge; ///< The signalbridge generates Qt signals from the PASM library so the user interface knows what to do
    QHash<uint32_t, QTableWidgetItem*> memoryTableMap; ///< Addresses are mapped to their specific table widget item
    QTableWidgetItem *nextToInterpret; ///< Next instruction/line to interpret
    QLabel *posInfo; ///< Position info label
    bool changedSinceSave; ///< Boolean to indicate whether the code has changed since the last save
    bool changedSinceCompile; ///< Boolean to indicate whether the code has changed since the last compile
    QString fileName; ///< Filename of the current file
    QString savePath, openPath; ///< Save path and open path
    Highlighter *highlighter; ///< Our syntax highlighting engine
    QTimer *timer;
    PasmThread *pasmThread;
    QList<int> bpList;
    QByteArray defaultLayout;
    int timedStepInterval;
    QString nameFilter;
    QTranslator *translator;

protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionSettings_triggered();
    void updatePalette();
    void enableMemoryEditFeatures(bool enable);
    void enableEditFeatures(bool enable);
    void toggleBreakpoint(int line);
    void updateRegisterInfo();
    void updateMemoryInfo(uint32_t address, int32_t value);
    void deleteMemoryCell(uint32_t address);
    void showErrorMessage(ErrorCode error);
    void readInput();
    void showOutput(int32_t value);
    void reset();
    void reportProgramStarted();
    void reportProgramEnded();
    void reportProgramPaused(bool paused);
    void updatePositionInfo();
    void updateCurrentLineMarker();
    void nextLineToInterpret(uint32_t address);
    void asmCodeTextChanged();
    void asmCodeModifiedChanged(bool changed);
    void enableUndoButton(bool enable);
    void enableRedoButton(bool enable);
    void setEditButtons();
    void setBreakpoint(uint32_t address, bool enabled);
    void memItemChanged(QTableWidgetItem* item);
    void setSplitterOrientationHorizontal();
    void setSplitterOrientationVertical();

    void on_flagZero_clicked();
    void on_flagNeg_clicked();
    void on_flagOverflow_clicked();
    void on_actionExit_triggered();
    void on_actionRun_triggered();
    void on_actionStep_triggered();
    void on_actionTimedStep_triggered();
    void on_actionPause_triggered();
    void on_actionStop_triggered();
    void on_actionCompile_triggered();
    void on_actionNew_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionOpen_triggered();
    void on_actionAboutQPasm_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_bpAddBtn_clicked();
    void on_bpDelBtn_clicked();
    void on_bpClearBtn_clicked();
    void on_checkEnableBp_toggled(bool checked);
    void on_lineRegA_textEdited(const QString &text);
    void on_lineRegB_textEdited(const QString &text);
    void on_linePC_textEdited(const QString &text);
    void on_memAddBtn_clicked();
    void on_memDelBtn_clicked();
    void on_memClearBtn_clicked();
    void on_splitter_customContextMenuRequested(QPoint pos);
};

#endif // MAINWINDOW_H

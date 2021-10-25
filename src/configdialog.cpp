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

#include "configdialog.h"
#include "ui_configdialog.h"
#include <QSettings>
#include <QColorDialog>
#include <QTranslator>

ConfigDialog::ConfigDialog(QWidget *parent, QTranslator *translator) :
    QDialog(parent),
    m_ui(new Ui::ConfigDialog) {
    m_ui->setupUi(this);
    main = qobject_cast<MainWindow*>(parent);
    showCustomColors(false);
    this->translator = translator;
    loadSettings();
    connect(m_ui->langBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLanguage(int)));
}

ConfigDialog::~ConfigDialog() {
    delete m_ui;
}

QIcon ConfigDialog::createIconFromColor(QColor color) {
    QPixmap pm(16, 16);
    pm.fill(color);
    return QIcon(pm);
}

void ConfigDialog::showCustomColors(bool show) {
    m_ui->pushData->setEnabled(show);
    m_ui->pushEmpty->setEnabled(show);
    m_ui->pushHalt->setEnabled(show);
    m_ui->pushInputOutput->setEnabled(show);
    m_ui->pushJump->setEnabled(show);
    m_ui->pushMath->setEnabled(show);
    m_ui->pushOperands->setEnabled(show);
    m_ui->pushLabels->setEnabled(show);
    m_ui->baseOnLabel->setEnabled(show);
    QString lightColText = (show ? " <a href=\"#light\">" : " ") + tr("Light colors") + (show ? "</a>" : "");
    QString darkColText = (show ? "<a href=\"#dark\">" : "") + tr("Dark colors") + (show ? "</a>" : "");
    m_ui->baseOnLabel->setText(tr("Base on:") + lightColText + " | " + darkColText);
}

void ConfigDialog::loadSettings() {
    QSettings settings("config.ini", QSettings::IniFormat, this);

    // TODO: replace with code which automatically lists all available translations
    m_ui->langBox->addItem(tr("English"), "");
    m_ui->langBox->addItem(tr("Dutch"), "nl");

    QString locale = settings.value("language", QLocale::system().name()).toString();
    for (int i = 0; i < m_ui->langBox->count(); ++i) {
        if (m_ui->langBox->itemData(i, Qt::UserRole).toString() == locale ||
            m_ui->langBox->itemData(i, Qt::UserRole).toString() == locale.left(2)) {
            m_ui->langBox->setCurrentIndex(i);
            break;
        }
    }

    m_ui->spinInterval->setValue(settings.value("timedStepInterval", 500).toInt());
    m_ui->fontComboBox->setCurrentFont(QFont(settings.value("font", "Monospace").toString()));

    QList<QVariant> defaultColors;
    defaultColors << QColor(Qt::darkBlue) << QColor(Qt::darkGray) << QColor(Qt::darkRed) << QColor(Qt::darkYellow)
            << QColor(Qt::darkMagenta) << QColor(Qt::darkGreen) << QColor(Qt::darkCyan) << QColor(Qt::blue);
    colors = settings.value("customColors", QVariant(defaultColors)).toList();
    if (colors.size() < 8)
        colors = defaultColors;
    setCustomColors(colors);

    QString scheme = settings.value("colorScheme", "automatic").toString();
    if (scheme == "automatic")
        m_ui->colorSchemeBox->setCurrentIndex(0);
    else if (scheme == "light")
        m_ui->colorSchemeBox->setCurrentIndex(1);
    else if (scheme == "dark")
        m_ui->colorSchemeBox->setCurrentIndex(2);
    else if (scheme == "custom") {
        m_ui->colorSchemeBox->setCurrentIndex(3);
        showCustomColors(true);
    }
}

void ConfigDialog::setCustomColors(const QList<QVariant> &colors) {
    if (colors.size() >= 8) {
        m_ui->pushData->setIcon(createIconFromColor(colors.at(0).value<QColor>()));
        m_ui->pushEmpty->setIcon(createIconFromColor(colors.at(1).value<QColor>()));
        m_ui->pushHalt->setIcon(createIconFromColor(colors.at(2).value<QColor>()));
        m_ui->pushInputOutput->setIcon(createIconFromColor(colors.at(3).value<QColor>()));
        m_ui->pushJump->setIcon(createIconFromColor(colors.at(4).value<QColor>()));
        m_ui->pushMath->setIcon(createIconFromColor(colors.at(5).value<QColor>()));
        m_ui->pushOperands->setIcon(createIconFromColor(colors.at(6).value<QColor>()));
        m_ui->pushLabels->setIcon(createIconFromColor(colors.at(7).value<QColor>()));
    }
}

void ConfigDialog::saveCustomColors(const QList<QVariant> &colors) {
    QSettings settings("config.ini", QSettings::IniFormat, this);
    settings.setValue("customColors", colors);
    main->updateColorScheme();
}

void ConfigDialog::changeEvent(QEvent *e) {
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this); // Retranslate the user interface on-the-fly
        break;
    default:
        break;
    }
}

void ConfigDialog::on_pushRestoreLayout_clicked() {
    main->restoreDefaultLayout();
    m_ui->pushRestoreLayout->setEnabled(false);
    m_ui->spinInterval->setFocus();
}

void ConfigDialog::on_spinInterval_valueChanged(int value) {
    main->setTimedStepInterval(value);
}

void ConfigDialog::on_fontComboBox_currentFontChanged(QFont font) {
    main->setEditorFont(font);
}

void ConfigDialog::on_colorSchemeBox_currentIndexChanged(int index) {
    QString scheme;
    switch (index) {
    case 0:
        scheme = "automatic";
        showCustomColors(false);
        break;
    case 1:
        scheme = "light";
        showCustomColors(false);
        break;
    case 2:
        scheme = "dark";
        showCustomColors(false);
        break;
    case 3:
        scheme = "custom";
        showCustomColors(true);
        break;
    }
    QSettings settings("config.ini", QSettings::IniFormat, this);
    settings.setValue("colorScheme", scheme);
    main->updateColorScheme();
}

void ConfigDialog::on_baseOnLabel_linkActivated(const QString &link) {
    QList<QVariant> colors;
    if (link == "#light") {
        colors << QColor(Qt::darkBlue).lighter(320) << QColor(Qt::lightGray) << QColor(Qt::darkRed).lighter(280) << QColor(Qt::darkYellow).lighter(280)
               << QColor(Qt::darkMagenta).lighter(280) << QColor(Qt::darkGreen).lighter(280) << QColor(Qt::white) << QColor(Qt::darkCyan).lighter(340);
    } else { // link == "#dark"
        colors << QColor(Qt::darkBlue) << QColor(Qt::darkGray) << QColor(Qt::darkRed) << QColor(Qt::darkYellow)
               << QColor(Qt::darkMagenta) << QColor(Qt::darkGreen) << QColor(Qt::darkCyan) << QColor(Qt::blue);
    }
    setCustomColors(colors);
    saveCustomColors(colors);
}

void ConfigDialog::pickColor(QPushButton *button, int index) {
    QColor color = QColorDialog::getColor(colors.size() >= index + 1 ? colors.at(index).value<QColor>() : Qt::black, this);
    if (color.isValid()) {
        colors[index] = color;
        button->setIcon(createIconFromColor(color));
        saveCustomColors(colors);
    }
}

void ConfigDialog::on_pushData_clicked() {
    pickColor(m_ui->pushData, 0);
}

void ConfigDialog::on_pushEmpty_clicked() {
    pickColor(m_ui->pushEmpty, 1);
}

void ConfigDialog::on_pushHalt_clicked() {
    pickColor(m_ui->pushHalt, 2);
}

void ConfigDialog::on_pushInputOutput_clicked() {
    pickColor(m_ui->pushInputOutput, 3);
}

void ConfigDialog::on_pushJump_clicked() {
    pickColor(m_ui->pushJump, 4);
}

void ConfigDialog::on_pushMath_clicked() {
    pickColor(m_ui->pushMath, 5);
}

void ConfigDialog::on_pushOperands_clicked() {
    pickColor(m_ui->pushOperands, 6);
}

void ConfigDialog::on_pushLabels_clicked() {
    pickColor(m_ui->pushLabels, 7);
}

void ConfigDialog::changeLanguage(int index) {
    QString locale = m_ui->langBox->itemData(index, Qt::UserRole).toString();
    if (!translator->load(QString(":i18n/qpasm_") + locale)) {
        qDebug() << "Warning: failed to load translation";
    }
    QSettings settings("config.ini", QSettings::IniFormat, this);
    settings.setValue("language", locale);
}

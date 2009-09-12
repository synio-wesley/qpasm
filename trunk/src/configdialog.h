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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
    class ConfigDialog;
}

class ConfigDialog : public QDialog {
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent = 0, QTranslator *translator = 0);
    ~ConfigDialog();

protected:
    void changeEvent(QEvent *e);

private:
    QIcon createIconFromColor(QColor color);
    void showCustomColors(bool show);
    void setCustomColors(const QList<QVariant> &colors);
    void saveCustomColors(const QList<QVariant> &colors);
    void loadSettings();
    void pickColor(QPushButton *button, int index);
    Ui::ConfigDialog *m_ui;
    MainWindow *main;
    QList<QVariant> colors;
    QTranslator *translator;

private slots:
    void changeLanguage(int index);
    void on_pushData_clicked();
    void on_pushEmpty_clicked();
    void on_pushHalt_clicked();
    void on_pushInputOutput_clicked();
    void on_pushJump_clicked();
    void on_pushMath_clicked();
    void on_pushOperands_clicked();
    void on_pushLabels_clicked();
    void on_baseOnLabel_linkActivated(const QString &link);
    void on_colorSchemeBox_currentIndexChanged(int index);
    void on_fontComboBox_currentFontChanged(QFont font);
    void on_spinInterval_valueChanged(int value);
    void on_pushRestoreLayout_clicked();
};

#endif // CONFIGDIALOG_H

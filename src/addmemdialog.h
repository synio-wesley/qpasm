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

#ifndef ADDMEMDIALOG_H
#define ADDMEMDIALOG_H

#include <QDialog>

namespace Ui {
    class AddMemDialog;
}

class AddMemDialog : public QDialog {
    Q_OBJECT
public:
    AddMemDialog(QWidget *parent = 0);
    ~AddMemDialog();
    void accept();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AddMemDialog *m_ui;

private slots:
    void on_lineAddress_textEdited(const QString &text);
    void on_lineDecValue_textEdited(const QString &text);
    void on_lineHexValue_textEdited(const QString &text);
    void on_lineBinValue_textEdited(const QString &text);
    void on_lineInstruction_textEdited(const QString &text);
};

#endif // ADDMEMDIALOG_H

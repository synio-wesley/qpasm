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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::AboutDialog) {
    m_ui->setupUi(this);
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

AboutDialog::~AboutDialog() {
    delete m_ui;
}

void AboutDialog::changeEvent(QEvent *e) {
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this); // Retranslate the user interface on-the-fly
        break;
    default:
        break;
    }
}

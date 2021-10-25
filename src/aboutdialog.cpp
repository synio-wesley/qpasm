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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#define VERSION_STRING "1.4"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::AboutDialog) {
    m_ui->setupUi(this);
    m_ui->labelVersion->setText(tr("<span style=\"font-size: 14pt;\">QPasm %1</span> (LibPASM %2)").arg(VERSION_STRING, VERSION_STRING));
    m_ui->labelAbout->setText(tr("QPasm is a graphical frontend for the PASM pseudo assembler library. You can write, compile, run and debug simple assembler programs with this application.") +
                              "<br /><br /><strong>" + tr("Developer:") + "</strong><br />" + "&nbsp;&nbsp;<em>Wesley Stessens</em> (<a href=\"mailto:wesley@ubuntu.com\">wesley@ubuntu.com</a>)" +
                              "<br /><strong>" + tr("Thanks to:") + "</strong><br />" + "&nbsp;&nbsp;<em>GyroW</em> (<a href=\"https://github.com/GyroW\">GyroW @ GitHub</a>)<br>" + "&nbsp;&nbsp;<em>Ward Segers</em> (<a href=\"https://github.com/editicalu\">editicalu @ GitHub</a>)" +
                              "<br /><strong>" + tr("Icons:") + "</strong><br />" + "&nbsp;&nbsp;<em>LGPL icons by KDE Breeze Team</em> (<a href=\"https://github.com/KDE/breeze-icons\">https://github.com/KDE/breeze-icons</a>)" +
                              "<br /><p align=\"center\"><em>" + tr("Developed for the course CPS/ICCS at Hasselt University") + "<br />&copy; Copyright 2009 Wesley Stessens</em></p>");
    m_ui->licenseText->setPlainText(tr("QPasm is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, "
                                       "either version 3 of the License, or (at your option) any later version.\n\n"
                                       "QPasm is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A "
                                       "PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.\n\n"
                                       "You should have received a copy of the GNU Lesser General Public License along with QPasm. If not, see <http://www.gnu.org/licenses/>."));
    m_ui->translatorsText->setText(tr("The following people have translated QPasm into other languages:") + "<br /><br />" +
                                   "<span style=\"font-size: 16px;\">" + tr("Dutch") + "</span>" + "<ul style=\"margin-top: 0px;\"><li><strong>cumulus007</strong>&nbsp;(<a href=\"mailto:cumulus0007@gmail.com\">cumulus0007@gmail.com</a>)</li></ul>");
    setFixedSize(sizeHint());
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

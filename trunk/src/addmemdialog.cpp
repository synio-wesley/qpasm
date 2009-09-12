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

#include "addmemdialog.h"
#include "ui_addmemdialog.h"
#include "linevalidator.h"
#include "libpasm/pasm.h"
#include <QMessageBox>

AddMemDialog::AddMemDialog(QWidget *parent) :
        QDialog(parent),
        m_ui(new Ui::AddMemDialog) {
    m_ui->setupUi(this);
    m_ui->addressExistsLabel->hide();
    m_ui->lineAddress->setValidator(new LineValidator(this, LineValidator::Unsigned32Bit, LineValidator::Decimal));
    m_ui->lineDecValue->setValidator(new LineValidator(this, LineValidator::Signed32Bit, LineValidator::Decimal));
    m_ui->lineHexValue->setValidator(new LineValidator(this, LineValidator::Signed32Bit, LineValidator::Hexadecimal));
    m_ui->lineBinValue->setValidator(new LineValidator(this, LineValidator::Signed32Bit, LineValidator::Binary));
}

AddMemDialog::~AddMemDialog()
{
    delete m_ui;
}

void AddMemDialog::accept() {
    if (m_ui->lineAddress->text().isEmpty()) {
        QMessageBox::information(this, tr("Address required"), tr("You need to choose an address to store the new data at."), QMessageBox::Ok, QMessageBox::Ok);
        m_ui->lineAddress->setFocus();
    } else {
        pasm_writeValue(m_ui->lineAddress->text().toUInt(), m_ui->lineDecValue->text().toInt());
        QDialog::accept();
    }
}

void AddMemDialog::changeEvent(QEvent *e) {
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this); // Retranslate the user interface on-the-fly
        break;
    default:
        break;
    }
}

void AddMemDialog::on_lineAddress_textEdited(const QString &text) {
    m_ui->addressExistsLabel->setVisible(!text.isEmpty() && pasm_memoryCellExists(text.toUInt()));
}

void AddMemDialog::on_lineDecValue_textEdited(const QString &text) {
    m_ui->lineHexValue->setText(QString::number(text.toInt(), 16));
    m_ui->lineBinValue->setText(QString::number(text.toInt(), 2));
    Instruction instruction = pasm_valueToInstruction(text.toInt());
    char *cmd = pasm_instructionToString(instruction);
    m_ui->lineInstruction->setText(cmd);
    m_ui->instructionLabel->setText(cmd);
    free(cmd);
}

void AddMemDialog::on_lineHexValue_textEdited(const QString &text) {
    m_ui->lineDecValue->setText(QString::number(text.toInt(NULL, 16)));
    m_ui->lineBinValue->setText(QString::number(text.toInt(NULL, 16), 2));
    Instruction instruction = pasm_valueToInstruction(text.toInt(NULL, 16));
    char *cmd = pasm_instructionToString(instruction);
    m_ui->lineInstruction->setText(cmd);
    m_ui->instructionLabel->setText(cmd);
    free(cmd);
}

void AddMemDialog::on_lineBinValue_textEdited(const QString &text) {
    m_ui->lineDecValue->setText(QString::number(text.toInt(NULL, 2)));
    m_ui->lineHexValue->setText(QString::number(text.toInt(NULL, 2), 16));
    Instruction instruction = pasm_valueToInstruction(text.toInt(NULL, 2));
    char *cmd = pasm_instructionToString(instruction);
    m_ui->lineInstruction->setText(cmd);
    m_ui->instructionLabel->setText(cmd);
    free(cmd);
}

void AddMemDialog::on_lineInstruction_textEdited(const QString &text) {
    QByteArray ba = text.toLatin1();
    ErrorCode error;
    char *unresolvedLabel = NULL;
    Instruction instruction = pasm_stringToInstruction(ba.data(), &error, NULL, &unresolvedLabel);

    if (error == NO_ERROR) {
        if (unresolvedLabel) {
            PendingItem *item = pasm_createPendingItem(0, unresolvedLabel);
            pasm_resolveItem(item, &instruction);
            free(item->label);
            free(item);
        }

        char *cmd = pasm_instructionToString(instruction);
        m_ui->instructionLabel->setText(cmd);
        free(cmd);
        int value = pasm_instructionToValue(instruction);
        m_ui->lineDecValue->setText(QString::number(value));
        m_ui->lineHexValue->setText(QString::number(value, 16));
        m_ui->lineBinValue->setText(QString::number(value, 2));
    } else {
        m_ui->instructionLabel->setText("NOP");
        m_ui->lineDecValue->setText("0");
        m_ui->lineHexValue->setText("0");
        m_ui->lineBinValue->setText("0");
    }
}

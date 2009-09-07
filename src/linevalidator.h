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

#ifndef LINEVALIDATOR_H
#define LINEVALIDATOR_H

#include <QValidator>

class LineValidator : public QValidator {
public:
    enum ValidatorType {Unsigned32Bit, Signed32Bit};
    enum InputFormat {Decimal, Hexadecimal, Binary};
    LineValidator(QObject *parent = 0, ValidatorType type = Signed32Bit, InputFormat format = Decimal);
    QValidator::State validate(QString &input, int &pos) const;
private:
    ValidatorType type;
    InputFormat format;
};

#endif // LINEVALIDATOR_H

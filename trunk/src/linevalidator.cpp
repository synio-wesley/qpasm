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

#include "linevalidator.h"

/* Maximum unsigned 32 bit number */
#define MAX_SIGNED_POSITIVE_32BIT_NUMBER  2147483647L
#define MAX_SIGNED_NEGATIVE_32BIT_NUMBER (-2147483647L - 1L)
#define MAX_UNSIGNED_32BIT_NUMBER         0xffffffffUL

LineValidator::LineValidator(QObject *parent, ValidatorType type, InputFormat format) : QValidator(parent) {
    this->type = type;
    this->format = format;
}

QValidator::State LineValidator::validate(QString &input, int &pos) const {
    Q_UNUSED(pos);
    bool ok = false;
    int base = format == Hexadecimal ? 16 : (format == Binary ? 2 : 10);

    if (type == Unsigned32Bit)
        ok = input.isEmpty() || (input.toUInt(&ok, base) <= MAX_UNSIGNED_32BIT_NUMBER && ok);
    else if (type == Signed32Bit) {
        ok = input.isEmpty() || (input.toInt(&ok, base) >= MAX_SIGNED_NEGATIVE_32BIT_NUMBER && ok) ||
             (input.toInt(&ok, base) <= MAX_SIGNED_POSITIVE_32BIT_NUMBER && ok);
    }
    return ok ? Acceptable : Invalid;
}

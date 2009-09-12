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

#include "listwidgetitem.h"

ListWidgetItem::ListWidgetItem(QListWidget *parent) : QListWidgetItem(parent) {
}

bool ListWidgetItem::operator< (const QListWidgetItem &other) const {
    return data(Qt::UserRole).toInt() < other.data(Qt::UserRole).toInt();
}
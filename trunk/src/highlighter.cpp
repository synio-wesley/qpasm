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

#include "highlighter.h"
#include <QTextCursor>

Highlighter::Highlighter(QTextDocument *parent, QTextEdit *edit) : QSyntaxHighlighter(parent) {
    this->edit = edit;
    darkBg = false;
    setColorScheme("automatic");
}

void Highlighter::setCustomColors(const QList<QVariant> &colors) {
    this->colors = colors;
}

void Highlighter::setColorScheme(const QString &scheme) {
    if (scheme == "automatic")
        darkBg = edit ? edit->palette().base().color().value() < 128 : false;
    else if (scheme == "light")
        darkBg = true;
    else if (scheme == "dark")
        darkBg = false;

    if (scheme != "custom" || colors.size() < 8) {
        colors.clear();
        colors << (darkBg ? QColor(Qt::darkBlue).lighter(320) : Qt::darkBlue);
        colors << (darkBg ? Qt::lightGray : Qt::darkGray);
        colors << (darkBg ? QColor(Qt::darkRed).lighter(280) : Qt::darkRed);
        colors << (darkBg ? QColor(Qt::darkYellow).lighter(280) : Qt::darkYellow);
        colors << (darkBg ? QColor(Qt::darkMagenta).lighter(280) : Qt::darkMagenta);
        colors << (darkBg ? QColor(Qt::darkGreen).lighter(280) : Qt::darkGreen);
        colors << (darkBg ? Qt::white : Qt::darkCyan);
        colors << (darkBg ? QColor(Qt::darkCyan).lighter(340) : Qt::blue);
    }
    rehighlight();
}

void Highlighter::highlightBlock(const QString &text) {
    QTextCharFormat format;
    QRegExp expression;
    QString pattern;

    expression.setCaseSensitivity(Qt::CaseInsensitive);

    // Operations to load or store data get a blue color (by default)
    format.setFontWeight(QFont::Bold);
    format.setForeground(colors.at(0).value<QColor>());
    pattern = "^\\s*([A-Z_]*:\\s*)?(LDA|LDB)\\s+(#-?\\d+|\\(\\d+\\)|\\d+|(?:\\[[A-Z_]*\\]|\\(\\[[A-Z_]*\\]\\)))(?:\\s|$)";
    expression.setPattern(pattern);
    apply(text, format, expression);
    pattern = "^\\s*([A-Z_]*:\\s*)?(STA|STB)\\s+(\\(\\d+\\)|\\d+|(?:\\[[A-Z_]*\\]|\\(\\[[A-Z_]*\\]\\)))(?:\\s|$)";
    expression.setPattern(pattern);
    apply(text, format, expression);

    // Jump operations get a magenta color (by default)
    format.setFontWeight(QFont::Bold);
    format.setForeground(colors.at(4).value<QColor>());
    pattern = "^\\s*([A-Z_]*:\\s*)?(?:(?:(JMP|JSP|JSN|JIZ|JOF|JSB)\\s+(\\d+|\\[[A-Z_]*\\]))|(RTS$|RTS\\s))";
    expression.setPattern(pattern);
    apply(text, format, expression);

    // An empty operation (NOP) gets a gray color (by default)
    format.setFontWeight(QFont::Bold);
    format.setForeground(colors.at(1).value<QColor>());
    pattern = "^\\s*([A-Z_]*:\\s*)?(NOP\\s|NOP$)";
    expression.setPattern(pattern);
    apply(text, format, expression);

    // Operations for input and output get a yellow color (by default)
    format.setFontWeight(QFont::Bold);
    format.setForeground(colors.at(3).value<QColor>());
    pattern = "^\\s*([A-Z_]*:\\s*)?(INP\\s|INP$|OUT\\s|OUT$)";
    expression.setPattern(pattern);
    apply(text, format, expression);

    // Mathematical operations get a green color (by default)
    format.setFontWeight(QFont::Bold);
    format.setForeground(colors.at(5).value<QColor>());
    pattern = "^\\s*([A-Z_]*:\\s*)?(ADD\\s|ADD$|SUB\\s|SUB$|MUL\\s|MUL$|DIV\\s|DIV$)";
    expression.setPattern(pattern);
    apply(text, format, expression);

    // The halt operation (HLT) gets a red color (by default)
    format.setFontWeight(QFont::Bold);
    format.setForeground(colors.at(2).value<QColor>());
    pattern = "^\\s*([A-Z_]*:\\s*)?(HLT\\s|HLT$)";
    expression.setPattern(pattern);
    apply(text, format, expression);
}

void Highlighter::apply(const QString &text, QTextCharFormat format, QRegExp expression) {
    text.indexOf(expression); // Try to match regular expression

    // Highlight label (blue color by default)
    if (expression.pos(2) >= 0) {
        QTextCharFormat labelFormat;
        labelFormat.setFontWeight(QFont::Bold);
        labelFormat.setForeground(colors.at(7).value<QColor>());
        setFormat(expression.pos(1), expression.cap(1).length(), labelFormat);
    }

    // Valid operands get a cyan or white color (by default)
    QTextCharFormat opFormat;
    opFormat.setFontWeight(QFont::Bold);
    opFormat.setForeground(colors.at(6).value<QColor>());
    setFormat(expression.pos(3), expression.cap(3).length(), opFormat);

    // Apply highlighting to operator
    setFormat(expression.pos(2), expression.cap(2).length(), format);
}

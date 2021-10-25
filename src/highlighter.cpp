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

#include "highlighter.h"
#include <QTextCursor>
#include <QRegularExpression>

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
        colors << QVariant::fromValue((darkBg ? QColor(Qt::darkBlue).lighter(320) : Qt::darkBlue));
        colors << QVariant::fromValue((darkBg ? Qt::lightGray : Qt::darkGray));
        colors << QVariant::fromValue((darkBg ? QColor(Qt::darkRed).lighter(280) : Qt::darkRed));
        colors << QVariant::fromValue((darkBg ? QColor(Qt::darkYellow).lighter(280) : Qt::darkYellow));
        colors << QVariant::fromValue((darkBg ? QColor(Qt::darkMagenta).lighter(280) : Qt::darkMagenta));
        colors << QVariant::fromValue((darkBg ? QColor(Qt::darkGreen).lighter(280) : Qt::darkGreen));
        colors << QVariant::fromValue((darkBg ? Qt::white : Qt::darkCyan));
        colors << QVariant::fromValue((darkBg ? QColor(Qt::darkCyan).lighter(340) : Qt::blue));
    }
    rehighlight();
}

void Highlighter::highlightBlock(const QString &text) {
    QTextCharFormat format;
    // TODO: remove
    QString pattern;

    // Operations to load or store data get a blue color (by default)
    format.setFontWeight(QFont::Bold);
    format.setForeground(colors.at(0).value<QColor>());
    
    QRegularExpression ldaLdbExpression("^\\s*([A-Z_]*:\\s*)?(LDA|LDB)\\s+(#-?\\d+|\\(\\d+\\)|\\d+|(?:\\[[A-Z_]*\\]|\\(\\[[A-Z_]*\\]\\)))(?:\\s|$)", QRegularExpression::CaseInsensitiveOption);
    apply(text, format, ldaLdbExpression);

    QRegularExpression staStbExpression("^\\s*([A-Z_]*:\\s*)?(STA|STB)\\s+(\\(\\d+\\)|\\d+|(?:\\[[A-Z_]*\\]|\\(\\[[A-Z_]*\\]\\)))(?:\\s|$)", QRegularExpression::CaseInsensitiveOption);
    apply(text, format, staStbExpression);

    // Jump operations get a magenta color (by default)
    format.setFontWeight(QFont::Bold);
    format.setForeground(colors.at(4).value<QColor>());
    QRegularExpression expression("^\\s*([A-Z_]*:\\s*)?(?:(?:(JMP|JSP|JSN|JIZ|JOF|JSB)\\s+(\\d+|\\[[A-Z_]*\\]))|(RTS$|RTS\\s))", QRegularExpression::CaseInsensitiveOption);
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

void Highlighter::apply(const QString &text, QTextCharFormat format, QRegularExpression& expression) {
    QRegularExpressionMatch match = expression.match(text); // Try to match regular expression

    // Highlight label (blue color by default)
    if (match.capturedStart(2) >= 0) {
        QTextCharFormat labelFormat;
        labelFormat.setFontWeight(QFont::Bold);
        labelFormat.setForeground(colors.at(7).value<QColor>());
        setFormat(match.capturedStart(1), match.captured(1).length(), labelFormat);
    }

    // Valid operands get a cyan or white color (by default)
    QTextCharFormat opFormat;
    opFormat.setFontWeight(QFont::Bold);
    opFormat.setForeground(colors.at(6).value<QColor>());
    setFormat(match.capturedStart(3), match.captured(3).length(), opFormat);

    // Apply highlighting to operator
    setFormat(match.capturedStart(2), match.captured(2).length(), format);
}

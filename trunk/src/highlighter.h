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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextEdit>

/**
  * Highlighter takes care of syntax highlighting in the application.
  * Inputted pseudo-assembler code will be highlighted automagically thanks to this class.
  */
class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    /**
      * Constructor.
      * \param parent Parent object
      */
    Highlighter(QTextDocument *parent = 0, QTextEdit *edit = 0);
    void setCustomColors(const QList<QVariant> &colors);
    void setColorScheme(const QString &scheme);

protected:
    /**
      * Highlight the given text block.
      * \param text Text block to test for highlighting
      */
    void highlightBlock(const QString &text);

private:
    /**
      * Apply highlighting.
      * \param text Text block to apply highlighting to
      * \param format Format of highlighting (color)
      * \param expression Expression that was matched
      */
    void apply(const QString &text, QTextCharFormat format, QRegExp expression);
    QTextEdit *edit;
    bool darkBg;
    QList<QVariant> colors;
};

#endif // HIGHLIGHTER_H

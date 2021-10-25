/*
    Copyright 2009 Wesley Stessens
    Copyright 2005, 2006 KJSEmbed Authors for part of the paintEvent() reimplementation

        Part of the paintEvent() reimplementation of the CodeEdit class was taken
        from the NumberedTextView class of the KJSEmbed project which is LGPL-licensed.

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

#include "codeedit.h"
#include <QHBoxLayout>
#include <QAbstractTextDocumentLayout>
#include <QScrollBar>
#include <QPainter>
#include <QTextBlock>
#include <QFontComboBox>

TextEdit::TextEdit(QWidget *parent) : QTextEdit(parent) {
    currentLine = -1;
    bpList = NULL;
    running = false;
    bpEnabled = false;
}

void TextEdit::setCurrentLine(int line) {
    currentLine = line;
    if (line >= 0 && line < document()->blockCount()) {
        QTextBlock block = document()->findBlockByNumber(line);
        QRectF boundingRect = document()->documentLayout()->blockBoundingRect(block);
        QPointF position = boundingRect.topLeft();
        int verticalScroll = verticalScrollBar()->value();
        if (position.y() < verticalScroll || position.y() + boundingRect.height() > verticalScroll + viewport()->height())
            verticalScrollBar()->setValue(position.y());
    }
    update();
    viewport()->update();
}

void TextEdit::setBreakpointsEnabled(bool enabled) {
    bpEnabled = enabled;
    update();
    viewport()->update();
}

void TextEdit::paintEvent(QPaintEvent *e) {
    if (running) {
        int lineCount = -1;
        QRectF boundingRect;
        int verticalScroll = verticalScrollBar()->value();
        for (QTextBlock block = document()->begin(); block.isValid(); block = block.next(), ++lineCount) {
            boundingRect = document()->documentLayout()->blockBoundingRect(block);
            QPointF position = boundingRect.topLeft();
            if (position.y() + boundingRect.height() < verticalScroll)
                continue;
            if (position.y() > verticalScroll + viewport()->height())
                break;
            const QString txt = QString::number(lineCount);
            if (lineCount + 1 == currentLine) {
                QPainter p(viewport());
                QLinearGradient grad(boundingRect.topLeft(), boundingRect.topRight());
                if (bpEnabled && bpList && bpList->contains(currentLine)) {
                    grad.setColorAt(0, QColor(255, 0, 0, this->palette().base().color().value() < 128 ? 85 : 35));
                    grad.setColorAt(1, QColor(255, 0, 0, 8));
                } else {
                    grad.setColorAt(0, QColor(0, 0, 255, this->palette().base().color().value() < 128 ? 85 : 35));
                    grad.setColorAt(1, QColor(0, 0, 255, 8));
                }
                p.fillRect(boundingRect.translated(0, -verticalScroll), grad);
                p.end();
                break;
            }
        }
    }
    QTextEdit::paintEvent(e);
}

NumberingWidget::NumberingWidget(QWidget *parent, TextEdit *edit) : QWidget(parent) {
    this->edit = edit;
    setFixedWidth(1);
    if (edit) {
        connect(edit->document()->documentLayout(), SIGNAL(update(const QRectF &)), this, SLOT(update()));
        connect(edit->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));
    }
}

void NumberingWidget::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);

    if (!edit)
        return;

    int verticalScroll = edit->verticalScrollBar()->value();
    const QFontMetrics fm = fontMetrics();
    int lineCount = 0, neededWidth = 1;

    QPainter p(this);
    for (QTextBlock block = edit->document()->begin(); block.isValid(); block = block.next(), ++lineCount) {
        const QRectF boundingRect = edit->document()->documentLayout()->blockBoundingRect(block);
        QPointF position = boundingRect.topLeft();
        if (position.y() + boundingRect.height() < verticalScroll)
            continue;
        if (position.y() > verticalScroll + edit->viewport()->height())
            break;
        const QString txt = QString::number(lineCount);
        neededWidth = qMax(neededWidth, fontMetrics().horizontalAdvance(txt));
        p.setPen(QColor(150, 150, 150));
        p.drawText(width() - fm.horizontalAdvance(txt) - 4, qRound(position.y()) + fontMetrics().ascent() - verticalScroll + 1, txt);
        if (edit->breakpointList() && edit->breakpointList()->contains(lineCount)) {
            if (!edit->breakpointsEnabled())
                p.setOpacity(0.4);
            p.drawPixmap(0, qRound(position.y()) - verticalScroll + 1, 16, 16, QPixmap(":/res/img/bp.svg"));
        }
        p.setOpacity(1.0);
    }

    setFixedWidth(neededWidth + 20 /* 4 + 16 */);
}

void NumberingWidget::mouseReleaseEvent(QMouseEvent *e) {
    qreal height = edit->document()->documentMargin();
    int lineCount = 0;
    int verticalScroll = edit->verticalScrollBar()->value();

    for (QTextBlock block = edit->document()->begin(); block.isValid(); block = block.next(), ++lineCount) {
        const QRectF boundingRect = edit->document()->documentLayout()->blockBoundingRect(block);
        if (e->position().y() > height - verticalScroll && e->position().y() < height - verticalScroll + boundingRect.height()) {
            emit lineClicked(lineCount);
            break;
        }
        height += boundingRect.height();
    }
}

CodeEdit::CodeEdit(QWidget *parent) : QFrame(parent) {
    edit = new TextEdit(this);
    edit->setFrameStyle(QFrame::NoFrame);
    
    numberingWidget = new NumberingWidget(this, edit);
    connect(numberingWidget, SIGNAL(lineClicked(int)), this, SIGNAL(lineClicked(int)));

    QFont font("Monospace");
    setNewFont(font);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(numberingWidget);
    layout->addWidget(edit);
    setLayout(layout);
}

void CodeEdit::setNewFont(QFont font) {
    edit->setFont(font);
    font.setBold(true);
    numberingWidget->setFont(font);
}

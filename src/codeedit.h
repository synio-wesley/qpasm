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

#ifndef CODEEDIT_H
#define CODEEDIT_H

#include <QWidget>
#include <QFrame>
#include <QTextCharFormat>
#include <QTextEdit>

class TextEdit : public QTextEdit {
    Q_OBJECT
public:
    TextEdit(QWidget *parent = 0);
    void setCurrentLine(int line);
    void setRunning(bool running) {this->running = running;}
    void setBreakpointList(QList<int> *bpList) {this->bpList = bpList;}
    QList<int> *breakpointList() {return bpList;}
    void setBreakpointsEnabled(bool enabled);
    bool breakpointsEnabled() {return bpEnabled;}
protected:
    void paintEvent(QPaintEvent *e);
private:
    int currentLine;
    QList<int> *bpList;
    bool bpEnabled;
    bool running;
};

class NumberingWidget : public QWidget {
    Q_OBJECT
public:
    NumberingWidget(QWidget *parent = 0, TextEdit *edit = 0);
signals:
    void lineClicked(int line);
protected:
    void paintEvent(QPaintEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
private:
    TextEdit *edit;
};

class CodeEdit : public QFrame {
    Q_OBJECT
public:
    CodeEdit(QWidget *parent = 0);
    QTextEdit *textEdit() const {return qobject_cast<QTextEdit*>(edit);}
public slots:
    void setBreakpointList(QList<int> *bpList) {edit->setBreakpointList(bpList);}
    void setCurrentLine(int line) {edit->setCurrentLine(line);}
    void setRunning(bool running) {edit->setRunning(running);}
    void setBreakpointsEnabled(bool enabled) {edit->setBreakpointsEnabled(enabled);
                                              numberingWidget->update();}
    void setNewFont(QFont font);
signals:
    void lineClicked(int line);
private:
    TextEdit *edit;
    NumberingWidget *numberingWidget;
};

#endif // CODEEDIT_H

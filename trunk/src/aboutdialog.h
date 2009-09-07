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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class AboutDialog;
}

/**
  * About dialog.
  */
class AboutDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(AboutDialog)
public:
    /**
      * Constructor.
      * \param parent Parent object
      */
    explicit AboutDialog(QWidget *parent = 0);

    /**
      * Destructor.
      */
    virtual ~AboutDialog();

protected:
    /**
      * There has been a state change.
      * \param e State being changed in this event
      */
    virtual void changeEvent(QEvent *e);

private:
    /**
      * User interface of the about dialog.
      */
    Ui::AboutDialog *m_ui;
};

#endif // ABOUTDIALOG_H

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

#ifndef QTSIGNALBRIDGE_H
#define QTSIGNALBRIDGE_H

#include <QObject>
#include <stdint.h>
#include "libpasm/ui.h"

/**
  * QtSignalBridge converts the C function callbacks by the PASM library to native Qt signals.
  * This way the Qt frontend can be written in Qt completely, without having to rely on C callbacks aside from this class.
  */
class QtSignalBridge : public QObject
{
    Q_OBJECT

public:
    /**
      * Constructor.
      * \param parent Parent object
      */
    QtSignalBridge(QObject *parent);

    /**
      * Register changed callback. This callback function is called when the register has changed.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      */
    static void cb_registerChanged(void *obj);

    /**
      * Memory changed callback. This callback function is called when the memory has changed.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      * \param address Virtual address that has changed
      * \param value New value in the virtual address
      */
    static void cb_memoryChanged(void *obj, uint32_t address, int32_t value);

    /**
      * Error message callback. This callback function is called when an error has occured.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      * \param error Error code of the error that has occured
      */
    static void cb_errorMessage(void *obj, ErrorCode error);

    /**
      * Read input request callback. This callback function is called when input is requested from the user.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      */
    static void cb_readInputRequested(void *obj);

    /**
      * Output available callback. This callback function is called when output is available.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      * \param value Outputted value
      */
    static void cb_outputAvailable(void *obj, int32_t value);

    /**
      * Interpreter reset callback. This callback function is called when the interpreter has been reset.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      */
    static void cb_interpreterReset(void *obj);

    /**
      * Program start callback. This callback function is called when a pseudo-assembler program has started successfully.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      */
    static void cb_programStart(void *obj);

    /**
      * Program end callback. This callback function is called when a pseudo-assembler program has ended.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      */
    static void cb_programEnd(void *obj);

    static void cb_programPaused(void *obj, Boolean paused);

    /**
      * Line interpreted callback. This callback function is called when a certain instruction has been interpreted.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      * \param address Virtual address of the instruction that has been interpreted
      */
    static void cb_lineInterpreted(void *obj, uint32_t address);

    /**
      * Next line callback. This callback function is called when the next line to be interpreted has changed.
      * \param obj Instantiated QtSignalBridge object that should be a child object of the main graphical user interface
      * \param address Virtual address of the instruction that will be interpreted next
      */
    static void cb_nextLine(void *obj, uint32_t address);

    static void cb_bpSet(void *obj, uint32_t address, Boolean enabled);

    static void cb_deleteMemCell(void *obj, uint32_t address);

    static void cb_rebootMePlz(void *obj);

signals:
    /**
      * Register has changed.
      */
    void registerChanged();

    /**
      * Memory has changed.
      * \param address Virtual address that has changed
      * \param value New value in the virtual address
      */
    void memoryChanged(uint32_t address, int32_t value);

    /**
      * Error has occured.
      * \param error Error code of the error that has occured
      */
    void errorMessage(ErrorCode error);

    /**
      * There is a request for input from the user.
      */
    void readInputRequested();

    /**
      * There is output available.
      * \param value Outputted value
      */
    void outputAvailable(int32_t value);

    /**
      * Interpreter has been reset.
      */
    void interpreterReset();

    /**
      * A pseudo-assembler program has started successfully.
      */
    void programStart();

    /**
      * A pseudo-assembler program has ended.
      */
    void programEnd();

    void programPaused(bool paused);

    /**
      * A certain instruction has been interpreted.
      * \param address Virtual address of the instruction that has been interpreted
      */
    void lineInterpreted(uint32_t address);

    /**
      * Next line to be interpreted.
      * \param address Virtual address of the instruction that will be interpreted next
      */
    void nextLine(uint32_t address);

    void breakpointSet(uint32_t address, bool enabled);

    void memoryCellDeleted(uint32_t address);

    void rebootMePlz();
};

#endif // QTSIGNALBRIDGE_H

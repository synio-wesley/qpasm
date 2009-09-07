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

#include "qtsignalbridge.h"

QtSignalBridge::QtSignalBridge(QObject *parent)
    : QObject(parent) {
    // Set the callback data to THIS instantiated QtSignalBridge object
    pasm_setCallbackData(this);

    // Register all the callbacks
    pasm_registerCallback(REGISTER_CHANGE,    cb_registerChanged);
    pasm_registerCallback(MEMORY_CHANGE,      (void(*)(void*))cb_memoryChanged);
    pasm_registerCallback(ERROR_MESSAGE,      (void(*)(void*))cb_errorMessage);
    pasm_registerCallback(READ_INPUT,         cb_readInputRequested);
    pasm_registerCallback(OUTPUT_AVAILABLE,   (void(*)(void*))cb_outputAvailable);
    pasm_registerCallback(INTERPRETER_RESET,  cb_interpreterReset);
    pasm_registerCallback(PROGRAM_START,      cb_programStart);
    pasm_registerCallback(PROGRAM_END,        cb_programEnd);
    pasm_registerCallback(PROGRAM_PAUSED,     (void(*)(void*))cb_programPaused);
    pasm_registerCallback(LINE_INTERPRETED,   (void(*)(void*))cb_lineInterpreted);
    pasm_registerCallback(NEXT_LINE,          (void(*)(void*))cb_nextLine);
    pasm_registerCallback(BREAKPOINT_SET,     (void(*)(void*))cb_bpSet);
    pasm_registerCallback(DELETE_MEMORY_CELL, (void(*)(void*))cb_deleteMemCell);
    pasm_registerCallback(REBOOT_ME_PLZ,      cb_rebootMePlz);
}

void QtSignalBridge::cb_registerChanged(void *obj) {
    static_cast<QtSignalBridge*>(obj)->emit registerChanged();
}

void QtSignalBridge::cb_memoryChanged(void *obj, uint32_t address, int32_t value) {
    static_cast<QtSignalBridge*>(obj)->emit memoryChanged(address, value);
}

void QtSignalBridge::cb_errorMessage(void *obj, ErrorCode error) {
    static_cast<QtSignalBridge*>(obj)->emit errorMessage(error);
}

void QtSignalBridge::cb_readInputRequested(void *obj) {
    static_cast<QtSignalBridge*>(obj)->emit readInputRequested();
}

void QtSignalBridge::cb_outputAvailable(void *obj, int32_t value) {
    static_cast<QtSignalBridge*>(obj)->emit outputAvailable(value);
}

void QtSignalBridge::cb_interpreterReset(void *obj) {
    static_cast<QtSignalBridge*>(obj)->emit interpreterReset();
}

void QtSignalBridge::cb_programStart(void *obj) {
    static_cast<QtSignalBridge*>(obj)->emit programStart();
}

void QtSignalBridge::cb_programEnd(void *obj) {
    static_cast<QtSignalBridge*>(obj)->emit programEnd();
}

void QtSignalBridge::cb_programPaused(void *obj, Boolean paused) {
    static_cast<QtSignalBridge*>(obj)->emit programPaused((bool)paused);
}

void QtSignalBridge::cb_lineInterpreted(void *obj, uint32_t address) {
    static_cast<QtSignalBridge*>(obj)->emit lineInterpreted(address);
}

void QtSignalBridge::cb_nextLine(void *obj, uint32_t address) {
    static_cast<QtSignalBridge*>(obj)->emit nextLine(address);
}

void QtSignalBridge::cb_bpSet(void *obj, uint32_t address, Boolean enabled) {
    static_cast<QtSignalBridge*>(obj)->emit breakpointSet(address, (bool)enabled);
}

void QtSignalBridge::cb_deleteMemCell(void *obj, uint32_t address) {
    static_cast<QtSignalBridge*>(obj)->emit memoryCellDeleted(address);
}

void QtSignalBridge::cb_rebootMePlz(void *obj) {
    static_cast<QtSignalBridge*>(obj)->emit rebootMePlz();
}

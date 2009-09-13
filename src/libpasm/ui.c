/*
    Copyright 2009 Wesley Stessens

    This file is part of LibPASM.

    LibPASM is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LibPASM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LibPASM. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ui.h"
#include "interpreter.h"
#include <stdlib.h>

extern Register reg; /* This provides access to the register */
CallbackList cbList; /* List of all callback functions */

void pasm_initUi() {
    /* Nullify these pointers */
    cbList.data = NULL;
    cbList.registerChange = NULL;
    cbList.memoryChange = NULL;
    cbList.errorMessage = NULL;
    cbList.readInput = NULL;
    cbList.outputAvailable = NULL;
    cbList.interpreterReset = NULL;
    cbList.programStart = NULL;
    cbList.programEnd = NULL;
    cbList.programPaused = NULL;
    cbList.lineInterpreted = NULL;
    cbList.nextLine = NULL;
    cbList.bpSet = NULL;
    cbList.deleteMemCell = NULL;
    cbList.rebootMePlz = NULL;
}

void pasm_setCallbackData(void *data) {
    cbList.data = data;
}

ErrorCode pasm_registerCallback(CallbackType type, void (*callBack)(void *data)) {
    switch (type) {
        case REGISTER_CHANGE:
            cbList.registerChange = callBack;
            break;
        case MEMORY_CHANGE:
            cbList.memoryChange = (void(*)(void*, uint32_t, int32_t))callBack;
            break;
        case ERROR_MESSAGE:
            cbList.errorMessage = (void(*)(void*, ErrorCode))callBack;
            break;
        case READ_INPUT:
            cbList.readInput = callBack;
            break;
        case OUTPUT_AVAILABLE:
            cbList.outputAvailable = (void(*)(void*, int32_t))callBack;
            break;
        case INTERPRETER_RESET:
            cbList.interpreterReset = callBack;
            break;
        case PROGRAM_START:
            cbList.programStart = callBack;
            break;
        case PROGRAM_END:
            cbList.programEnd = callBack;
            break;
        case PROGRAM_PAUSED:
            cbList.programPaused = (void(*)(void*, Boolean))callBack;
            break;
        case LINE_INTERPRETED:
            cbList.lineInterpreted = (void(*)(void*, uint32_t))callBack;
            break;
        case NEXT_LINE:
            cbList.nextLine = (void(*)(void*, uint32_t))callBack;
            break;
        case BREAKPOINT_SET:
            cbList.bpSet = (void(*)(void*, uint32_t, Boolean))callBack;
            break;
        case DELETE_MEMORY_CELL:
            cbList.deleteMemCell = (void(*)(void*, uint32_t))callBack;
            break;
        case REBOOT_ME_PLZ:
            cbList.rebootMePlz = callBack;
            break;
        default:
            pasm_error(UNSUPPORTED_CALLBACK);
            return UNSUPPORTED_CALLBACK;
    }
    return NO_ERROR;
}

void pasm_performCallback(CallbackType type) {
    switch (type) {
        case REGISTER_CHANGE:
            if (cbList.registerChange)
                cbList.registerChange(cbList.data);
            break;
        case READ_INPUT:
            if (cbList.readInput)
                cbList.readInput(cbList.data);
            break;
        case INTERPRETER_RESET:
            if (cbList.interpreterReset)
                cbList.interpreterReset(cbList.data);
            break;
        case PROGRAM_START:
            if (cbList.programStart)
                cbList.programStart(cbList.data);
            break;
        case PROGRAM_END:
            if (cbList.programEnd)
                cbList.programEnd(cbList.data);
            break;
        case REBOOT_ME_PLZ:
            if (cbList.rebootMePlz)
                cbList.rebootMePlz(cbList.data);
        case PROGRAM_PAUSED:
        case MEMORY_CHANGE:
        case ERROR_MESSAGE:
        case OUTPUT_AVAILABLE:
        case LINE_INTERPRETED:
        case NEXT_LINE:
        case BREAKPOINT_SET:
        case DELETE_MEMORY_CELL:
            break;
    }
}

void pasm_performBpSetCallback(uint32_t address, Boolean enabled) {
    if (cbList.bpSet)
        cbList.bpSet(cbList.data, address, enabled);
}

void pasm_performPauseCallback(Boolean paused) {
    if (cbList.programPaused)
        cbList.programPaused(cbList.data, paused);
}

void pasm_performMemoryCallback(uint32_t address, int32_t value) {
    if (cbList.memoryChange)
        cbList.memoryChange(cbList.data, address, value);
}

void pasm_performOutputCallback(int32_t value) {
    if (cbList.outputAvailable)
        cbList.outputAvailable(cbList.data, value);
}

void pasm_performCurrentLineCallback(uint32_t address) {
    if (cbList.lineInterpreted)
        cbList.lineInterpreted(cbList.data, address);
}

void pasm_performNextLineCallback(uint32_t address) {
    if (cbList.nextLine)
        cbList.nextLine(cbList.data, address);
}

void pasm_performDeleteMemCallback(uint32_t address) {
    if (cbList.deleteMemCell)
        cbList.deleteMemCell(cbList.data, address);
}

void pasm_error(ErrorCode errorCode) {
    if (cbList.errorMessage)
        cbList.errorMessage(cbList.data, errorCode);
}

void pasm_readInput() {
    pasm_performCallback(READ_INPUT);
}

void pasm_setInput(int32_t input) {
    reg.a = input;
    pasm_resumeInterpreter();
}

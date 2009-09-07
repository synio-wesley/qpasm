/*
    Copyright 2009 Wesley Stessens

    This file is part of LibPASM.

    LibPASM is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LibPASM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LibPASM. If not, see <http://www.gnu.org/licenses/>.
*/

#include "errorcodes.h"
#include "helper.h"
#include "ui.h"
#include <string.h>
#include <stdlib.h>

ErrorList list;

void pasm_initErrorList() {
    list.firstError = NULL;
    list.lastError = NULL;
}

void pasm_resetErrors() {
    Error *error = list.firstError, *errorToDelete;
    while (error) {
        errorToDelete = error;
        if (error->symbol)
            free(error->symbol);
        error = error->nextError;
        free(errorToDelete);
    }
    pasm_initErrorList();
}

void pasm_addError(ErrorCode type, uint32_t address, const char *symbol) {
    Error *error = (Error*)malloc(sizeof(Error));
    Boolean cancel = _FALSE;
    if (error == NULL) {
        pasm_error(HOST_MEMORY_EXHAUSTED);
        cancel = _TRUE;
    } else {
        if (type == INVALID_MEMORY_REGION || type == DIVISION_BY_ZERO || type == OVERFLOW) {
            error->address = address;
            error->symbol = NULL;
        } else if (type == UNRESOLVABLE_SYMBOL) {
            char *label = (char*)malloc(strlen(symbol) + 1);
            if (label == NULL)
                pasm_error(HOST_MEMORY_EXHAUSTED);
            else {
                strcpy(label, symbol);
                error->symbol = label;
                error->address = address;
            }
        } else {
            free(error);
            cancel = _TRUE;
        }
    }

    if (!cancel) {
        error->nextError = NULL;
        error->prevError = list.lastError;
        error->type = type;
        if (list.lastError) /* This is not the first error in the list */
            list.lastError->nextError = error;
        else
            list.firstError = list.lastError = error;
    }
}

Error *pasm_popError() {
    Error *error = list.firstError;
    if (list.firstError)
        list.firstError = list.firstError->nextError;
    if (list.firstError == NULL)
        list.lastError = NULL;
    return error;
}

void pasm_freeError(Error *error) {
    if (error->symbol)
        free(error->symbol);
    free(error);
}

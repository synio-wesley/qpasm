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

#include "debugger.h"
#include "ui.h"
#include <stdlib.h>

Breakpoint *breakpoint;
Boolean debuggerEnabled;

void pasm_initDebugger() {
    breakpoint = NULL;
    debuggerEnabled = _FALSE;
}

void pasm_enableDebugger(Boolean enable) {
    debuggerEnabled = enable;
}

void pasm_setBreakpoint(uint32_t address, Boolean enable) {
    Breakpoint *bp = breakpoint, *prevBp = breakpoint;
    Boolean found = _FALSE;
    while (!found && bp) {
        if (bp->address == address) {
            found = _TRUE;
        } else {
            prevBp = bp;
            bp = bp->nextBp;
        }
    }
    if (found && !enable) {
        if (prevBp == bp)
            breakpoint = bp->nextBp;
        else
            prevBp->nextBp = bp->nextBp;
        free(bp);
         pasm_performBpSetCallback(address, _FALSE);
    } else if (!found && enable) {
        if (prevBp == NULL)
            bp = breakpoint = (Breakpoint*)malloc(sizeof(Breakpoint));
        else {
            prevBp->nextBp = (Breakpoint*)malloc(sizeof(Breakpoint));
            bp = prevBp->nextBp;
        }
        bp->nextBp = NULL;
        bp->address = address;
        bp->skip = _FALSE;
        pasm_performBpSetCallback(address, _TRUE);
    }
}

Boolean pasm_hasBreakpoint(uint32_t address) {
    Breakpoint *bp = breakpoint;
    Boolean found = _FALSE;

    if (!debuggerEnabled)
        return _FALSE;

    while (!found && bp) {
        if (bp->address == address) {
            if (bp->skip) {
                bp->skip = _FALSE;
                return _FALSE;
            }
            found = _TRUE;
            bp->skip = _TRUE;
        }
        bp = bp->nextBp;
    }
    return found;
}

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

#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include <stdint.h>
#include "helper.h"

typedef struct breakpoint {
    uint32_t address;
    Boolean skip;
    struct breakpoint *nextBp;
} Breakpoint;

#ifdef __cplusplus
extern "C" {
#endif
void pasm_initDebugger();
void pasm_enableDebugger(Boolean enable);
void pasm_setBreakpoint(uint32_t address, Boolean enable);
Boolean pasm_hasBreakpoint(uint32_t address);
#ifdef __cplusplus
}
#endif

#endif

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

#ifndef _ERRORCODES_H_
#define _ERRORCODES_H_

#include <stdint.h>

/**
 * Errors that can be triggered in the PASM library.
 * \file errorcodes.h
 * \author Wesley Stessens
 */

/**
  * Possible errors that can occur.
  */
typedef enum {
    NO_ERROR, /**< No error has occured */
    HOST_MEMORY_EXHAUSTED, /**< Memory of the host machine has been exhausted during dynamic memory allocation */
    UNSUPPORTED_OPCODE, /**< The operator code seems to be unsupported */
    UNSUPPORTED_ADDRESSMODE, /**< The address mode seems to be unsupported */
    UNSUPPORTED_CALLBACK, /**< The callback function seems to be unsupported */
    OPERAND_REQUIRED, /**< An operand was required for this instruction but was not given */
    INVALID_MEMORY_REGION, /**< The pseudo-assembler program tried to access an invalid (virtual) memory region */
    CALLSTACK_UNDERFLOW, /**< The pseudo-assembler program tried to return from a subroutine, but the callstack seems to be empty */
    DIVISION_BY_ZERO, /**< The pseudo-assembler program tried to divide the value in register A by zero */
    UNRESOLVABLE_SYMBOL, /**< A symbol couldn't be resolved */
    OVERFLOW
} ErrorCode;

typedef struct error {
    ErrorCode type;
    uint32_t address;
    char *symbol;
    struct error *prevError;
    struct error *nextError;
} Error;

typedef struct {
    Error *firstError;
    Error *lastError;
} ErrorList;

#ifdef __cplusplus
extern "C" {
#endif
void pasm_initErrorList();
void pasm_resetErrors();
void pasm_addError(ErrorCode type, uint32_t address, const char *symbol);
Error *pasm_popError();
void pasm_freeError(Error *error);
#ifdef __cplusplus
}
#endif

#endif

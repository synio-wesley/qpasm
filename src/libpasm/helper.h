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

#ifndef _HELPER_H_
#define _HELPER_H_

/**
 * Helper functionality for all other files.
 * \file helper.h
 * \author Wesley Stessens
 */

#include "errorcodes.h"
#include <stdint.h>

/** Address mode: no addressing */
#define NO_ADDRESSING        0x0
/** Address mode: immediate addressing */
#define IMMEDIATE_ADDRESSING 0x0
/** Address mode: direct addressing */
#define DIRECT_ADDRESSING    0x1
/** Address mode: indirect addressing */
#define INDIRECT_ADDRESSING  0x2

/** Value for the operand when no operand has been given */
#define NO_OPERAND           0xffffff

/**
  * Boolean type.
  * \note To avoid conflicts with other libraries, the values for FALSE and TRUE are prepended with an underscore.
  */
typedef enum {
    _FALSE = 0, /**< Value for false */
    _TRUE = 1 /**< Value for true */
} Boolean;

/**
  * Representation of an instruction in an easily accessible structure. You can use the pasm_valueToInstruction() function
  * to convert a value into this structure which is easier to work with than the raw value is.
  * \sa pasm_stringToInstruction
  * \sa pasm_instructionToString
  * \sa pasm_instructionToValue
  * \sa pasm_valueToInstruction
  */
typedef struct {
    unsigned int opCode : 6; /**< Operator code of the instruction */
    unsigned int addressMode : 2; /**< Address mode of the instruction */
    unsigned int operand : 24; /**< Operand of the instruction */
} Instruction;

#ifdef __cplusplus
extern "C" {
#endif
/**
  * Compile a string into an instruction.
  * \note This function changes the line variable. Be careful.
  * \param line String representation of an instruction to compile into an actual instruction
  * \param error An error code (NO_ERROR if no error occured)
  * \param myLabel Label gets set to the label that refers to this instruction (or NULL if there is no label)
  * \param unresolvedLabel If this instruction points to another instruction by using a label, unresolvedLabel will get set to this label (or NULL if there is no label)
  * \return The compiled instruction
  */
Instruction pasm_stringToInstruction(char *line, ErrorCode *error, char **myLabel, char **unresolvedLabel);

/**
  * Get a string representation of an instruction.
  * \param instruction Instruction to get a string representation for
  * \return String representation of the instruction
  * \note The string returned by this function must be freed after you are done using it!
  */
char *pasm_instructionToString(Instruction instruction);

/**
  * Convert an instruction to its numerical value.
  * \param instruction Instruction to convert
  * \return Numerical value of the instruction
  */
uint32_t pasm_instructionToValue(Instruction instruction);

/**
  * Convert a numerical value to an instruction.
  * \param value Numerical value to convert
  * \return Instruction that is compiled from the value
  */
Instruction pasm_valueToInstruction(int32_t value);

/**
  * Convert an unsigned value into a signed value.
  * \param unsignedValue The unsigned value
  * \return Signed value
  */
int32_t pasm_getSignedValue(uint32_t unsignedValue);

/**
  * Convert a signed value into an unsigned value.
  * \param signedValue The signed value
  * \return Unsigned value
  */
uint32_t pasm_getUnsignedValue(int32_t signedValue);
#ifdef __cplusplus
}
#endif

#endif

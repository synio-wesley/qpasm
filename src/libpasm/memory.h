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

#ifndef _MEMORY_H_
#define _MEMORY_H_

/**
 * Memory system of the pseudo-assembler. The memory system takes care of reading values from and writing values into the virtual memory.
 * The memory system knows 2 ways to allocate virtual memory: virtual static memory and virtual dynamic memory. Virtual static memory is
 * created all at once and allows for fast address seeking. Virtual dynamic memory gets created when it is needed but traversing the virtual
 * dynamic memory is a lot slower.
 * \file memory.h
 * \author Wesley Stessens
 */

#include "interpreter.h"

typedef int32_t MemCell; /**< Virtual memory cell content */

/**
  * Dynamic memory cell. This type of memory cell belongs to the virtual dynamic memory.
  */
typedef struct dynamicMemCell {
    uint32_t address; /**< Virtual address of the dynamic memory cell */
    MemCell *content; /**< Virtual memory cell content */
    struct dynamicMemCell *next; /**< Another virtual dynamic memory cell (they are linked together by a linked list) */
} DynamicMemCell;

/**
  * Interface to the dynamic memory.
  */
typedef struct {
    DynamicMemCell *start; /**< First dynamic memory cell */
    DynamicMemCell *end; /**< Last dynamic memory cell */
} DynamicMemory;

/**
  * Static memory.
  */
typedef struct {
    uint32_t size; /**< Size of the static memory */
    MemCell *start; /**< Starting address of the static memory */
} StaticMemory;

#ifdef __cplusplus
extern "C" {
#endif
/**
  * Initialize the memory system.
  */
void pasm_initMemory();

/**
  * Reset the memory system. This will also free all memory that was allocated on the host machine for the virtual memory.
  */
void pasm_resetMemory();

/**
  * Write some value into some virtual memory cell.
  * \param address Virtual address to write the value into
  * \param value Value to write into the memory cell
  * \note If you want to write an instruction instead, use pasm_writeInstruction()
  * \sa pasm_writeInstruction
  * \sa pasm_readValue
  */
void pasm_writeValue(uint32_t address, int32_t value);

/**
  * Read value from a virtual memory cell.
  * \param address Virtual address to read the value from
  * \return Value in the virtual memory cell
  * \note If the virtual memory cell does not exist, the INVALID_MEMORY_REGION error will get triggered
  *       and the pseudo-assembler program will stop.
  * \sa pasm_writeValue
  */
int32_t pasm_readValue(uint32_t address);

Boolean pasm_isDynamicMemory(uint32_t address);
Boolean pasm_memoryCellExists(uint32_t address);
Boolean pasm_deleteMemoryCell(uint32_t address);

/**
  * Create a block of virtual static memory.
  * \param size Size of the static memory.
  */
void pasm_createStaticMemory(uint32_t size);

/**
  * Write some instruction into some virtual memory cell.
  * \param address Virtual address to write the instruction into
  * \param instruction Instruction to write into the memory cell
  * \note If you want to write a value instead, use pasm_writeValue()
  * \sa pasm_writeValue
  * \sa pasm_readInstruction
  */
void pasm_writeInstruction(uint32_t address, Instruction instruction);

/**
  * Read instruction from a virtual memory cell.
  * \param address Virtual address to read the instruction from
  * \param instruction Pointer to empty instruction. The function will fill in the instruction.
  * \return An error code (NO_ERROR if no error occured)
  * \note If the virtual memory cell does not exist, the INVALID_MEMORY_REGION error will get triggered.
  * \sa pasm_writeInstruction
  */
ErrorCode pasm_readInstruction(uint32_t address, Instruction *instruction);
#ifdef __cplusplus
}
#endif

#endif

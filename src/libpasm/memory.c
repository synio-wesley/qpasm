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

#include "memory.h"
#include "ui.h"
#include "helper.h"
#include <stdlib.h>
#include <stdint.h>

StaticMemory staticMemory; /* Virtual static memory */
DynamicMemory dynamicMemory; /* Virtual dynamic memory */

/**
  * Allocate a dynamic memory cell.
  * \param address Virtual address to allocate
  * \return Allocated memory cell
  */
static MemCell *pasm_allocateDynamicMem(uint32_t address);

/**
  * Seek a dynamic memory cell.
  * \param address Virtual address to find
  * \return Found memory cell, or NULL if the memory cell could not be found
  */
static MemCell *pasm_seekDynamicMemCell(uint32_t address);

void pasm_initMemory() {
    staticMemory.size = 0;
    /* Nullify pointers */
    staticMemory.start = NULL;
    dynamicMemory.start = NULL;
    dynamicMemory.end = NULL;
}

void pasm_resetMemory() {
    DynamicMemCell *dynamicMemCell = dynamicMemory.start;
    DynamicMemCell *toDelete;

    // Free all dynamically allocated host memory
    while (dynamicMemCell != NULL) {
        toDelete = dynamicMemCell;
        dynamicMemCell = dynamicMemCell->next;
        free(toDelete->content);
        free(toDelete);
    }
    free(staticMemory.start);
    pasm_initMemory();
}

static MemCell *pasm_allocateDynamicMem(uint32_t address) {
    DynamicMemCell *dynamicMemCell = (DynamicMemCell*)malloc(sizeof(DynamicMemCell));
    MemCell *memCell = (MemCell*)malloc(sizeof(MemCell));

    if (memCell == NULL || dynamicMemCell == NULL)
        pasm_error(HOST_MEMORY_EXHAUSTED);
    else {
        dynamicMemCell->address = address;
        dynamicMemCell->content = memCell;
        dynamicMemCell->next = NULL;

        if (dynamicMemory.start == NULL) { /* This is the first dynamically allocated virtual memory cell */
            dynamicMemory.start = dynamicMemCell;
            dynamicMemory.end = dynamicMemCell;
        } else { /* This is not the first dynamically allocated virtual memory cell */
            dynamicMemory.end->next = dynamicMemCell;
            dynamicMemory.end = dynamicMemCell;
        }
    }
    return memCell;
}

MemCell *pasm_seekDynamicMemCell(uint32_t address) {
    DynamicMemCell *cell = dynamicMemory.start;
    MemCell *cellContent = NULL;
    Boolean found = _FALSE;

    while (!found && cell) {
        if (cell->address == address) {
            cellContent = cell->content;
            found = _TRUE;
        } else
            cell = cell->next;
    }
    return cellContent;
}

void pasm_writeValue(uint32_t address, int32_t value) {
    MemCell *memCell;

    if (address < staticMemory.size) { /* Is the address to write into part of the virtual static memory? */
        *(staticMemory.start + address) = value;
    } else { /* Is the address to write into part of the existing virtual dynamic memory? */
        memCell = pasm_seekDynamicMemCell(address);
        if (memCell == NULL)
            memCell = pasm_allocateDynamicMem(address); /* Allocate some new virtual dynamic memory */
        *memCell = value;
    }
    pasm_performMemoryCallback(address, value);
}

int32_t pasm_readValue(uint32_t address) {
    int32_t value = 0;
    MemCell *memCell;

    if (address < staticMemory.size) { /* Is the address to read part of the virtual static memory? */
        value = *(staticMemory.start + address);
    } else { /* Is the address to read part of the existing virtual dynamic memory? */
        memCell = pasm_seekDynamicMemCell(address);
        if (memCell != NULL)
            value = *memCell;
        else { /* The requested virtual address does not exist */
            pasm_addError(INVALID_MEMORY_REGION, address, NULL);
            pasm_error(INVALID_MEMORY_REGION);
            pasm_stopProgram();
        }
    }
    return value;
}

Boolean pasm_isDynamicMemory(uint32_t address) {
    return address >= staticMemory.size;
}

Boolean pasm_memoryCellExists(uint32_t address) {
    if (address < staticMemory.size)
        return _TRUE;
    else if (pasm_seekDynamicMemCell(address))
        return _TRUE;
    return _FALSE;
}

Boolean pasm_deleteMemoryCell(uint32_t address) { /* ret _FALSE if cell is static */
    if (pasm_memoryCellExists(address)) {
        if (address < staticMemory.size)
            return _FALSE;
        else {
            DynamicMemCell *memCell = dynamicMemory.start, *prevMemCell = dynamicMemory.start;
            Boolean found = _FALSE;
            while (!found && memCell) {
                if (memCell->address == address) {
                    found = _TRUE;
                } else {
                    prevMemCell = memCell;
                    memCell = memCell->next;
                }
            }
            if (found) {
                if (prevMemCell == memCell)
                    dynamicMemory.start = memCell->next;
                else
                    prevMemCell->next = memCell->next;
                free(memCell);
                pasm_performDeleteMemCallback(address);
            }
        }
    }
    return _TRUE;
}

void pasm_createStaticMemory(uint32_t size) {
    staticMemory.size = size;
    staticMemory.start = (MemCell*)calloc(size, sizeof(MemCell)); /* calloc instead of malloc because calloc also initializes the memory to 0 */
    if (staticMemory.start == NULL && size > 0)
        pasm_error(HOST_MEMORY_EXHAUSTED);
}

void pasm_writeInstruction(uint32_t address, Instruction instruction) {
    pasm_writeValue(address, pasm_instructionToValue(instruction));
}

ErrorCode pasm_readInstruction(uint32_t address, Instruction *instruction) {
    MemCell *memCell;
    ErrorCode error = NO_ERROR;

    if (address >= staticMemory.size) { /* Is the address to read part of the existing virtual dynamic memory? */
        memCell = pasm_seekDynamicMemCell(address);
        if (memCell == NULL) { /* The requested virtual address does not exist */
            error = INVALID_MEMORY_REGION;
            pasm_addError(error, address, NULL);
        } else
            *instruction = pasm_valueToInstruction(*memCell);
    } else /* The address to read is part of the virtual static memory */
        *instruction = pasm_valueToInstruction(*(staticMemory.start + address));
    return error;
}

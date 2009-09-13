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

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

/**
 * Pseudo-assembler interpreter. The interpreter is the core of the library.
 * It is mainly used to initialize the library and to start, run and stop pseudo-assembler programs.
 * \file interpreter.h
 * \author Wesley Stessens
 */

#include "errorcodes.h"
#include "helper.h"
#include "opcodes.h"
#include <stdint.h>

/* Maximum signed positive 32 bit number */
#define MAX_SIGNED_POSITIVE_32BIT_NUMBER 0x7fffffffL
/* Minimum signed negative 32 bit number */
#define MIN_SIGNED_NEGATIVE_32BIT_NUMBER (-MAX_SIGNED_POSITIVE_32BIT_NUMBER - 1L)

/** Register of the pseudo-assembler. */
typedef struct {
    int32_t a; /**< A register */
    int32_t b; /**< B register */
    uint32_t programCounter; /**< Program counter */
    Boolean zeroFlag; /**< Zero flag */
    Boolean negativeFlag; /**< Negative flag */
    Boolean overflowFlag; /**< Overflow flag */
} Register;

/** An element in the call stack. */
typedef struct stackElement {
    uint32_t address; /**< Address of the current element */
    struct stackElement *under; /**< The element below the current element */
} StackElement;

/** The call stack. */
typedef struct {
    StackElement *top; /**< The top element in the call stack */
} CallStack;

/** The jump mode to use. */
typedef enum {
    JUMP_TO_ADDRESS, /**< Jump to addresses */
    JUMP_TO_LINE /**< Jump to line numbers */
} JumpMode;

/** Label (symbol) that has been resolved */
typedef struct symbol {
    char *label;
    uint32_t resolved;
    struct symbol *nextSymbol;
} Symbol;

/** Pending address containing label to be resolved */
typedef struct pendingItem {
    uint32_t address;
    char *label;
    struct pendingItem *nextItem;
} PendingItem;

/** Symbol resolver (to resolve labels into addresses). */
typedef struct {
    Symbol *firstSymbol;
    PendingItem *firstPendingItem;
} SymbolResolver;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Initialize the PASM library.
 * \note You need to do this before calling any other function in the library.
 */
void pasm_init();

/**
  * Free all allocated memory that is still dynamically allocated by the PASM library.
  * \note You need to call this when you want to stop using the PASM library.
  */
void pasm_free();

/**
  * Reset the PASM library. All resources will be freed and the GUI will be sent the INTERPRETER_RESET signal.
  */
void pasm_reset();

/**
  * Set the jump mode.
  * \param mode Jump mode
  */
void pasm_setJumpMode(JumpMode mode);

/**
  * Get the state of the register.
  * \return Copy of the register state
  */
Register pasm_getRegister();

void pasm_setRegister(Register newReg);

PendingItem *pasm_createPendingItem(uint32_t address, const char *label);
PendingItem *pasm_addPendingItem(uint32_t address, const char *label);
Boolean pasm_resolveItem(PendingItem *item, Instruction *simulated);
void pasm_freeSymbolData(Boolean freeSymbols);

/**
  * Interpret a pseudo-assembler instruction.
  * \param instruction Instruction to interpret
  * \return An error code (NO_ERROR if no error occured)
  */
ErrorCode pasm_interpretInstruction(Instruction instruction);

/**
  * Compile a pseudo-assembler program.
  * \param program The program to compile. Each instruction needs to be on a seperate line. Newlines must be used to seperate the lines.
  * \param size The number of instructions (or lines) of the pseudo-assembler program.
  * \note You don't necessarily have to know the right value for the size parameter. The PASM library will use the value to determine how much
  *       virtual static memory to create because this will allow for faster data seeking. If the size is smaller, the remaining instructions
  *       will be allocated as virtual dynamic memory instead.
  * \return An error code (NO_ERROR if no error occured)
  */
ErrorCode pasm_compileProgram(const char *program, uint32_t size);

/**
  * Interpret the next instruction. If you want to run the whole pseudo-assembler program at once, consider using the pasm_runProgram() function.
  * \sa pasm_runProgram
  */
void pasm_step();

/**
  * Run the whole pseudo-assembler program at once. If you want to step through the program step by step, consider using the pasm_step() function.
  * \sa pasm_step()
  */
void pasm_runProgram();

void pasm_pauseProgram();

void pasm_resumeInterpreter();

/**
  * Stop the current pseudo-assembler program.
  */
void pasm_stopProgram();
#ifdef __cplusplus
}
#endif

#endif

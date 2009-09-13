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

#include "interpreter.h"
#include "memory.h"
#include "ui.h"
#include "debugger.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

Register reg; /* Register of pseudo-assembler */
CallStack callStack; /* Callstack of pseudo-assembler (needed for subroutines) */
Boolean programRunning; /* Boolean to determine whether a pseudo-assembler program is running */
Boolean programPaused;
Boolean completeRun;
JumpMode jumpMode; /* Jump mode */
uint32_t interpretedLineNumber;
SymbolResolver symbolResolver; /* Symbol resolver (to resolve labels into addresses) */

/**
  * Initialize the register.
  */
static void pasm_initRegister();

/**
  * Push a virtual address onto the call stack.
  * \param address Virtual address to push onto the call stack.
  */
static void pasm_pushOnCallStack(uint32_t address);

/**
  * Pop a virtual address from the call stack.
  * \param error An error code (NO_ERROR if no error occured)
  * \return Virtual address that was popped from the call stack.
  */
static uint32_t pasm_popFromCallStack(ErrorCode *error);

/**
  * Reset the call stack. This will also free the data that was allocated for the call stack.
  */
static void pasm_resetCallStack();

static void pasm_addSymbol(const char *label, uint32_t resolved);
static Boolean pasm_resolveSymbols();

static void pasm_performDelayedCallbacks(uint32_t interpretedLineNumber);
static void pasm_updateRegister();

static void pasm_addSymbol(const char *label, uint32_t resolved) {
    Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
    if (symbol == NULL)
        pasm_error(HOST_MEMORY_EXHAUSTED);
    else {
        char *newLabel = (char*)malloc(strlen(label) + 1);
        if (newLabel == NULL)
            pasm_error(HOST_MEMORY_EXHAUSTED);
        else {
            strcpy(newLabel, label);
            symbol->label = newLabel;
            symbol->resolved = resolved;
            symbol->nextSymbol = NULL;
        }
    }

    if (symbolResolver.firstSymbol) /* This is not the first symbol in the list */
        symbol->nextSymbol = symbolResolver.firstSymbol;
    symbolResolver.firstSymbol = symbol;
}

PendingItem *pasm_createPendingItem(uint32_t address, const char *label) {
    PendingItem *item = (PendingItem*)malloc(sizeof(PendingItem));
    if (item) {
        char *newLabel = (char*)malloc(strlen(label) + 1);
        if (newLabel == NULL)
            pasm_error(HOST_MEMORY_EXHAUSTED);
        else {
            strcpy(newLabel, label);
            item->address = address;
            item->label = newLabel;
            item->nextItem = NULL;
        }
    }
    return item;
}

PendingItem *pasm_addPendingItem(uint32_t address, const char *label) {
    PendingItem *item = pasm_createPendingItem(address, label);
    if (item == NULL)
        pasm_error(HOST_MEMORY_EXHAUSTED);
    else {
        if (symbolResolver.firstPendingItem) /* This is not the first pending item in the list */
            item->nextItem = symbolResolver.firstPendingItem;
        symbolResolver.firstPendingItem = item;
    }
    return item;
}

Boolean pasm_resolveItem(PendingItem *item, Instruction *simulated) {
    Symbol *symbol;
    ErrorCode error = NO_ERROR;
    Instruction instruction;
    Boolean found = _FALSE;
    symbol = symbolResolver.firstSymbol;

    while (symbol) {
        if (strcmp(item->label, symbol->label) == 0) {
            if (found) /* Same label points to multiple addresses */
                return _FALSE;
            if (simulated)
                simulated->operand = symbol->resolved;
            else {
                error = pasm_readInstruction(item->address, &instruction);
                if (error != NO_ERROR)
                    return _FALSE;
                instruction.operand = symbol->resolved;
                pasm_writeInstruction(item->address, instruction);
            }
            found = _TRUE;
        }
        symbol = symbol->nextSymbol;
    }
    if (!found) /* Couldn't find symbol */
        return _FALSE;
    return _TRUE;
}

static Boolean pasm_resolveSymbols() { /* returns _TRUE on success */
    PendingItem *item = symbolResolver.firstPendingItem;
    while (item) {
        if (!pasm_resolveItem(item, NULL)) {
            /* pasm_error() will be triggered in compile function */
            pasm_addError(UNRESOLVABLE_SYMBOL, item->address, item->label);
            return _FALSE;
        }
        item = item->nextItem;
    }
    return _TRUE;
}

void pasm_freeSymbolData(Boolean freeSymbols) {
    Symbol *symbol = symbolResolver.firstSymbol, *symbolToDelete;
    PendingItem *item = symbolResolver.firstPendingItem, *itemToDelete;
    if (freeSymbols) {
        while (symbol) {
            symbolToDelete = symbol;
            free(symbol->label);
            symbol = symbol->nextSymbol;
            free(symbolToDelete);
        }
        symbolResolver.firstSymbol = NULL;
    }
    while (item) {
        itemToDelete = item;
        free(item->label);
        item = item->nextItem;
        free(itemToDelete);
    }
    symbolResolver.firstPendingItem = NULL;
}

void pasm_init() {
    /* Initialize some other things first */
    pasm_initErrorList();
    pasm_initUi();
    pasm_initMemory();
    pasm_initRegister();
    jumpMode = JUMP_TO_ADDRESS;

    /* Nullify call stack pointer */
    callStack.top = NULL;
}

void pasm_free() {
    pasm_stopProgram();
    pasm_resetMemory();
    pasm_resetCallStack();
    pasm_resetErrors();
    /* Init symbol resolver */
    pasm_freeSymbolData(_TRUE);
    symbolResolver.firstSymbol = NULL;
    symbolResolver.firstPendingItem = NULL;
}

void pasm_reset() {
    pasm_free();
    pasm_initRegister();
    pasm_performCallback(INTERPRETER_RESET);
    completeRun = _FALSE;
}

void pasm_pushOnCallStack(uint32_t address) {
    StackElement *element = (StackElement*)malloc(sizeof(StackElement));
    if (element == NULL)
        pasm_error(HOST_MEMORY_EXHAUSTED);
    else {
        element->address = address;
        element->under = callStack.top;
        callStack.top = element;
    }
}

uint32_t pasm_popFromCallStack(ErrorCode *error) {
    StackElement *element;
    uint32_t address = 0;

    if (callStack.top == NULL)
        *error = CALLSTACK_UNDERFLOW;
    else {
        element = callStack.top;
        address = element->address;
        callStack.top = element->under;
        free(element);
    }
    return address;
}

void pasm_resetCallStack() {
    StackElement *element = callStack.top;
    StackElement *toDelete;
    while (element != NULL) {
        toDelete = element;
        element = element->under;
        free(toDelete);
    }
    callStack.top = NULL;
}

static void pasm_initRegister() {
    reg.a = 0;
    reg.b = 0;
    reg.programCounter = 0;
    reg.zeroFlag = _TRUE;
    reg.overflowFlag = _FALSE;
    reg.negativeFlag = _FALSE;
}


Register pasm_getRegister() {
    return reg;
}

void pasm_setRegister(Register newReg) {
    reg = newReg;
}

void pasm_setJumpMode(JumpMode mode) {
    if (!programRunning) /* Jump mode cannot be changed while running a program */
        jumpMode = mode;
}

ErrorCode pasm_interpretInstruction(Instruction instruction) {
    ErrorCode error = NO_ERROR;
    int64_t int64;

    switch (instruction.opCode) {
        case NOP:
            break;
        case LDA:
        case LDB:
            reg.overflowFlag = _FALSE;
            int32_t *regItem = instruction.opCode == LDA ? &(reg.a) : &(reg.b);
            if (instruction.addressMode == IMMEDIATE_ADDRESSING)
                *regItem = pasm_getSignedValue(instruction.operand);
            else if (instruction.addressMode == DIRECT_ADDRESSING)
                *regItem = pasm_readValue(instruction.operand);
            else if (instruction.addressMode == INDIRECT_ADDRESSING)
                *regItem = pasm_readValue(pasm_readValue(instruction.operand));
            else
                error = UNSUPPORTED_ADDRESSMODE;
            break;
        case STA:
        case STB:
            if (instruction.addressMode == DIRECT_ADDRESSING)
                pasm_writeValue(instruction.operand, instruction.opCode == STA ? reg.a : reg.b);
            else if (instruction.addressMode == INDIRECT_ADDRESSING) {
                int32_t address = pasm_readValue(instruction.operand);
                if (address < 0) {
                    error = INVALID_MEMORY_REGION;
                    pasm_addError(error, address, NULL);
                } else
                    pasm_writeValue(address, instruction.opCode == STA ? reg.a : reg.b);
            } else
                error = UNSUPPORTED_ADDRESSMODE;
            break;
        case INP:
            reg.overflowFlag = _FALSE;
            programRunning = _FALSE; /* Pause the interpreter - needed when the frontend runs this library in a seperate thread */
            pasm_performDelayedCallbacks(interpretedLineNumber);
            pasm_readInput();
            break;
        case OUT:
            pasm_performOutputCallback(reg.a);
            break;
        case ADD:
            reg.overflowFlag = _FALSE;
            int64 = (int64_t)reg.a + reg.b;
            if (int64 > MAX_SIGNED_POSITIVE_32BIT_NUMBER || int64 < MIN_SIGNED_NEGATIVE_32BIT_NUMBER) {
                reg.overflowFlag = _TRUE;
                pasm_addError(OVERFLOW, reg.programCounter, NULL);
                pasm_error(OVERFLOW);
            }
            /*if (reg.a < 0 && reg.b < 0 && reg.a + reg.b > reg.a) // Experimental overflow detection which does not require a 64 bit int but has not been tested thoroughly
                reg.overflowFlag = _TRUE;
            else if (reg.a >= 0 && reg.b >= 0 && reg.a + reg.b < reg.a)
                reg.overflowFlag = _TRUE;*/
            reg.a += reg.b;
            break;
        case SUB:
            reg.overflowFlag = _FALSE;
            int64 = (int64_t)reg.a - reg.b;
            if (int64 > MAX_SIGNED_POSITIVE_32BIT_NUMBER || int64 < MIN_SIGNED_NEGATIVE_32BIT_NUMBER) {
                reg.overflowFlag = _TRUE;
                pasm_addError(OVERFLOW, reg.programCounter, NULL);
                pasm_error(OVERFLOW);
            }
            /*if (reg.a < 0 && reg.b >= 0 && reg.a - reg.b > reg.a) // Experimental overflow detection which does not require a 64 bit int but has not been tested thoroughly
                reg.overflowFlag = _TRUE;
            else if (reg.a >= 0 && reg.b < 0 && reg.a - reg.b < reg.a)
                reg.overflowFlag = _TRUE;*/
            reg.a -= reg.b;
            break;
        case MUL:
            reg.overflowFlag = _FALSE;
            int64 = (int64_t)reg.a * reg.b;
            if (int64 > MAX_SIGNED_POSITIVE_32BIT_NUMBER || int64 < MIN_SIGNED_NEGATIVE_32BIT_NUMBER) {
                reg.overflowFlag = _TRUE;
                pasm_addError(OVERFLOW, reg.programCounter, NULL);
                pasm_error(OVERFLOW);
            }
            reg.a *= reg.b;
            break;
        case DIV:
            reg.overflowFlag = _FALSE;
            if (reg.a == MIN_SIGNED_NEGATIVE_32BIT_NUMBER && reg.b == -1) {
                reg.overflowFlag = _TRUE;
                pasm_addError(OVERFLOW, reg.programCounter, NULL);
                pasm_error(OVERFLOW);
            } else if (reg.b == 0) {
                error = DIVISION_BY_ZERO;
                pasm_addError(error, reg.programCounter, NULL);
            } else
                reg.a /= reg.b;
            break;
        case JMP:
            reg.programCounter = instruction.operand - (jumpMode == JUMP_TO_ADDRESS ? 1 : 2);
            break;
        case JSP:
            if (!reg.negativeFlag && !reg.zeroFlag)
                reg.programCounter = instruction.operand - (jumpMode == JUMP_TO_ADDRESS ? 1 : 2);
            break;
        case JSN:
            if (reg.negativeFlag)
                reg.programCounter = instruction.operand - (jumpMode == JUMP_TO_ADDRESS ? 1 : 2);
            break;
        case JIZ:
            if (reg.zeroFlag)
                reg.programCounter = instruction.operand - (jumpMode == JUMP_TO_ADDRESS ? 1 : 2);
            break;
        case JOF:
            if (reg.overflowFlag)
                reg.programCounter = instruction.operand - (jumpMode == JUMP_TO_ADDRESS ? 1 : 2);
            break;
        case JSB:
            pasm_pushOnCallStack(reg.programCounter);
            reg.programCounter = instruction.operand - (jumpMode == JUMP_TO_ADDRESS ? 1 : 2);
            break;
        case RTS:
            reg.programCounter = pasm_popFromCallStack(&error);
            break;
        case HLT:
            break; /* The assembler program will automatically halt on the HLT instruction
                      or when an error occured; this is handled in pasm_step() */
        default:
            error = UNSUPPORTED_OPCODE;
    }
    return error;
}

static void pasm_performDelayedCallbacks(uint32_t interpretedLineNumber) {
    pasm_performCallback(REGISTER_CHANGE);
    pasm_performNextLineCallback(reg.programCounter);
    pasm_performCurrentLineCallback(interpretedLineNumber);
}

static void pasm_updateRegister() {
    reg.programCounter++;
    reg.zeroFlag = reg.a == 0 ? _TRUE : _FALSE;
    reg.negativeFlag = reg.a < 0 ? _TRUE : _FALSE;
}

void pasm_resumeInterpreter() {
    programRunning = _TRUE;
    pasm_performCurrentLineCallback(reg.programCounter);
    pasm_updateRegister();
    pasm_performNextLineCallback(reg.programCounter);
    pasm_performCallback(REGISTER_CHANGE);
    if (completeRun)
        pasm_performCallback(REBOOT_ME_PLZ);
}

ErrorCode pasm_compileProgram(const char *program, uint32_t size) {
    const char *start = program, *end = NULL;
    char *line = NULL, *myLabel = NULL, *unresolvedLabel = NULL;
    uint32_t count = 0;
    ErrorCode error = NO_ERROR;

    pasm_reset();
    pasm_createStaticMemory(size);

    /* This loop will run through the program code line by line and will compile instructions
       and put these instructions into the virtual memory */
    while (*start != '\0' && (error == NO_ERROR || error == UNSUPPORTED_OPCODE || error == UNSUPPORTED_ADDRESSMODE || error == OPERAND_REQUIRED)) {
        end = strchr(start, '\n');
        if (end == NULL)
            end = strchr(start, '\0');
        line = (char*)malloc(end - start + 1);
        memcpy(line, start, end - start);
        line[end - start] = '\0';
        pasm_writeInstruction(count, pasm_stringToInstruction(line, &error, &myLabel, &unresolvedLabel));

        if (myLabel) /* There is a label attached to this address */
            pasm_addSymbol(myLabel, count);
        if (unresolvedLabel) /* This instruction refers to another address by using a label */
            pasm_addPendingItem(count, unresolvedLabel);

        count++;
        free(line);
        if (*end == '\0')
            start = end;
        else
            start = end + 1;
    }

    /* Let's resolve the unresolved symbols! */
    if (!pasm_resolveSymbols())
        error = UNRESOLVABLE_SYMBOL;
    pasm_freeSymbolData(_FALSE);

    programRunning = _FALSE;
    programPaused = _FALSE;
    pasm_initRegister();
    pasm_performCallback(REGISTER_CHANGE);

    /* Invalid instructions get translated to NOP so it doesn't matter */
    if (error != NO_ERROR && error != UNSUPPORTED_OPCODE && error != UNSUPPORTED_ADDRESSMODE && error != OPERAND_REQUIRED) {
        pasm_error(error);
        pasm_reset();
    }
    return error;
}

void pasm_step() {
    Instruction instruction;
    ErrorCode error;

    if (pasm_hasBreakpoint(reg.programCounter)) {
        pasm_pauseProgram();
        return;
    }

    if (programPaused) {
        programRunning = _TRUE;
        programPaused = _FALSE;
        pasm_performPauseCallback(_FALSE);
    }

    if (!programRunning) {
        pasm_initRegister();
        pasm_performCallback(PROGRAM_START);
        programRunning = _TRUE;
    }

    interpretedLineNumber = reg.programCounter;
    error = pasm_readInstruction(reg.programCounter, &instruction);
    if (error == NO_ERROR) {
        error = pasm_interpretInstruction(instruction);
        if (instruction.opCode != HLT && instruction.opCode != INP) {
            pasm_updateRegister();
            if (!completeRun) {
                pasm_performCallback(REGISTER_CHANGE);
                pasm_performNextLineCallback(reg.programCounter);
                pasm_performCurrentLineCallback(interpretedLineNumber);
            }
        }
        if (completeRun) { /* THis is done to prevent the GUI from freezing when there is heavy processing or an endless loop */
            static clock_t timer = 0;
            clock_t now = clock();
            if (now - timer >= CLOCKS_PER_SEC / 20) {
                pasm_performDelayedCallbacks(interpretedLineNumber);
                timer = clock();
            }
        }
    }
    if (error != NO_ERROR)
        pasm_error(error);
    if (error != NO_ERROR || instruction.opCode == HLT)
        pasm_stopProgram();
}

void pasm_runProgram() {
    completeRun = _TRUE;
    do {
        pasm_step();
    } while (programRunning);
}

void pasm_pauseProgram() {
    if (programRunning) {
        pasm_performDelayedCallbacks(interpretedLineNumber);
        programRunning = _FALSE;
        programPaused = _TRUE;
        completeRun = _FALSE;
        pasm_performPauseCallback(_TRUE);
    }
}

void pasm_stopProgram() {
    if (programRunning || programPaused) {
        pasm_performDelayedCallbacks(interpretedLineNumber);
        programRunning = _FALSE;
        programPaused = _FALSE;
        pasm_performCallback(PROGRAM_END);
    }
    completeRun = _FALSE;
}

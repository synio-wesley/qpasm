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

#ifndef _UI_H_
#define _UI_H_

/**
 * User interface system. The user interface system is the glue between the PASM library and the actual graphical user interface frontend.
 * \file ui.h
 * \author Wesley Stessens
 */

#include "errorcodes.h"
#include "helper.h"
#include <stdint.h>

/**
  * Type of callback. These are the possible callbacks that a frontend can register.
  */
typedef enum {
    REGISTER_CHANGE, /**< Register has changed */
    MEMORY_CHANGE, /**< Memory has changed */
    ERROR_MESSAGE, /**< Error has occured */
    READ_INPUT, /**< Input is required */
    OUTPUT_AVAILABLE, /**< Output is available */
    INTERPRETER_RESET, /**< Interpreter has been reset */
    PROGRAM_START, /**< Program has started in the pseudo-assembler */
    PROGRAM_END, /**< Program has ended in the pseudo-assembler */
    PROGRAM_PAUSED,
    LINE_INTERPRETED, /**< A certain instruction has been interpreted */
    NEXT_LINE,
    BREAKPOINT_SET,
    DELETE_MEMORY_CELL,
    REBOOT_ME_PLZ
} CallbackType;

/**
  * List of callback pointers. This structure holds pointers to functions in the GUI.
  * These functions get called when the PASM library wants to notify the GUI of something.
  */
typedef struct {
    void *data; /**< Optional data that the GUI receives as an argument when a callback is called. */
    void (*registerChange)(void *data); /**< Callback function to call when the register has changed. */
    void (*memoryChange)(void *data, uint32_t address, int32_t value); /**< Callback function to call when the memory has changed. */
    void (*errorMessage)(void *data, ErrorCode errorCode); /**< Callback function to call when an error has occured. */
    void (*readInput)(void *data); /**< Callback function to call when input is required. */
    void (*outputAvailable)(void *data, int32_t value); /**< Callback function to call when output is available. */
    void (*interpreterReset)(void *data); /**< Callback function to call when the interpreter has been reset. */
    void (*programStart)(void *data); /**< Callback function to call when a program has started in the pseudo-assembler. */
    void (*programEnd)(void *data); /**< Callback function to call when a program has ended in the pseudo-assembler. */
    void (*programPaused)(void *data, Boolean paused);
    void (*lineInterpreted)(void *data, uint32_t address); /**< Callback function to call when a certain instruction has been interpreted. */
    void (*nextLine)(void *data, uint32_t address); /**< Callback function to notify the UI which line will be interpreted next */
    void (*bpSet)(void *data, uint32_t address, Boolean enabled);
    void (*deleteMemCell)(void *data, uint32_t address);
    void (*rebootMePlz)(void *data);
} CallbackList;

#ifdef __cplusplus
extern "C" {
#endif
/**
  * Initialize the user interface system.
  */
void pasm_initUi();

/**
  * Set data that will be used as one of the arguments when a callback GUI function is called.
  * \param data The data that will be used as one of the arguments. This is useful in some special cases,
                for example when you are writing a C++ based frontend; you could write static callback functions
                and to interact with the main object/window you could provide that object as a parameter.
  */
void pasm_setCallbackData(void *data);

/**
  * Register a callback function with the library.
  * \param type Type of callback
  * \param callBack The callback function
  * \return error that occured when registering the callback (NO_ERROR or UNSUPPORTED_CALLBACK)
  */
ErrorCode pasm_registerCallback(CallbackType type, void (*callBack)(void *));

/**
  * Perform a simple callback function of the GUI. You use this to actually notify the GUI frontend. To notify the GUI about a changed memory address
  * or to notify the GUI that there is output available or that a certain instruction has been interpreted or that an error has occured,
  * you will need to use other functions.
  * \param type Type of callback
  * \sa pasm_performMemoryCallback
  * \sa pasm_performOutputCallback
  * \sa pasm_performLineCallback
  * \sa pasm_error
  */
void pasm_performCallback(CallbackType type);

void pasm_performBpSetCallback(uint32_t address, Boolean enabled);
void pasm_performPauseCallback(Boolean paused);

/**
  * Notify the GUI that a memory address has changed through a callback function of the GUI.
  * \param address Virtual address that has changed
  * \param value New value in the virtual address
  */
void pasm_performMemoryCallback(uint32_t address, int32_t value);

/**
  * Notify the GUI that there is output available through a callback function of the GUI.
  * \param value Outputted value
  */
void pasm_performOutputCallback(int32_t value);

/**
  * Notify the GUI that a certain instruction has been interpreted through a callback function of the GUI.
  * \param address Virtual address of the instruction that has been interpreted
  */
void pasm_performCurrentLineCallback(uint32_t address);

/**
  * Notify the GUI which line will be interpreted next.
  * \param address Virtual address of the instruction that will be interpreted next
  */
void pasm_performNextLineCallback(uint32_t address);

void pasm_performDeleteMemCallback(uint32_t address);

/**
  * Notify the GUI that an error has occured through a callback function of the GUI.
  * \param errorCode An error code (NO_ERROR if no error occured)
  */
void pasm_error(ErrorCode errorCode);

/**
  * Request input from the GUI through a callback function of the GUI. The GUI has to call pasm_setInput() to actually set the input value.
  * \note pasm_performCallback(READ_INPUT) does the same thing. This is just a convenience function.
  */
void pasm_readInput();

/**
  * Set the input value of the assembler. Usually this function is called from the GUI when input is required.
  * \note You can register the READ_INPUT callback function with the PASM library to know when input is required.
  * \param input Inputted value
  */
void pasm_setInput(int32_t input);
#ifdef __cplusplus
}
#endif

#endif

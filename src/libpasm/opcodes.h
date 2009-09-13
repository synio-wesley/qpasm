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

#ifndef _OPCODES_H_
#define _OPCODES_H_

/**
 * All operator codes for the pseudo-assembler.
 * \file opcodes.h
 * \author Wesley Stessens
 */

/**
  * Operator codes for the pseudo-assembler.
  */
typedef enum {
    NOP = 0x00, /**< NOP instruction; does nothing */

    LDA = 0x04, /**< LDA instruction; loads something into register A */
    LDB = 0x05, /**< LDB instruction; loads something into register B */
    STA = 0x08, /**< STA instruction; stores the content of register A in a virtual address */
    STB = 0x09, /**< STB instruction; stores the content of register B in a virtual address */

    INP = 0x18, /**< INP instruction; ask for input and store the input in register A */
    OUT = 0x1C, /**< OUT instruction; output the value in register A */

    ADD = 0x20, /**< ADD instruction; store the value in register A + the value in register B in register A */
    SUB = 0x21, /**< SUB instruction; store the value in register A - the value in register B in register A */
    MUL = 0x28, /**< MUL instruction; store the value in register A * the value in register B in register A */
    DIV = 0x2C, /**< DIV instruction; store the value in register A / the value in register B in register A */

    JMP = 0x30, /**< JMP instruction; unconditionally jump to some line (or address) */
    JSP = 0x32, /**< JSP instruction; jump to some line (or address) when the value in register A is strictly positive */
    JSN = 0x34, /**< JSN instruction; jump to some line (or address) when the value in register A is strictly negative */
    JIZ = 0x36, /**< JIZ instruction; jump to some line (or address) when the value in register A is equal to 0 */
    JOF = 0x38, /**< JOF instruction; jump to some line (or address) when an overflow occured */

    JSB = 0x3C, /**< JSB instruction; unconditionally enter some subroutine */
    RTS = 0x3D, /**< RTS instruction; return back from the subroutine */

    HLT = 0x3F /**< HLT instruction; stop the program */
} OpCode;

#endif

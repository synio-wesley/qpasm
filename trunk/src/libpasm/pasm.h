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

#ifndef _PASM_H_
#define _PASM_H_

/**
  * \mainpage PASM (Pseudo-ASseMbler) Library
  * The PASM Library provides a simple pseudo-assembler interpreter.<br />
  * This library provides the building blocks for creating a small Integrated Development Environment for small pseudo-assembler programs.<br /><br />
  * PASM was developed by Wesley Stessens in 2009 in assignment of Hasselt University.
  */

/**
 * Collection of needed PASM header files for frontends. Frontends will need to include this file.
 * \file pasm.h
 * \author Wesley Stessens
 */

#include "interpreter.h"
#include "memory.h"
#include "ui.h"
#include "errorcodes.h"
#include "opcodes.h"
#include "helper.h"
#include "debugger.h"

#endif

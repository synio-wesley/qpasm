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

#include "helper.h"
#include "opcodes.h"
#include "ui.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>

/* Maximum signed positive 24 bit number */
#define MAX_SIGNED_POSITIVE_24BIT_NUMBER 0x7fffff
/* Maximum unsigned 24 bit number */
#define MAX_UNSIGNED_24BIT_NUMBER        0xffffff

static char *uppercase(char *str);
static int needsOperand(OpCode opCode);
static Boolean validLabel(char *start, char *end);

static char *uppercase(char *str) {
    while (str && *str != '\0') {
        *str = toupper((unsigned char)*str);
        ++str;
    }
    return str;
}

static int needsOperand(OpCode opCode) {
    return opCode == LDA || opCode == LDB || opCode == STA ||
           opCode == STB || opCode == JSB || opCode == JMP || opCode == JSP ||
           opCode == JSN || opCode == JIZ || opCode == JOF;
}

static Boolean validLabel(char *start, char *end) {
    Boolean validLabel = _TRUE;
    char *c;
    for (c = start; validLabel && c <= end; ++c) {
        if (!((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || *c == '_')) /* Only [a-Z_] are supported in labels */
            validLabel = _FALSE;
    }
    return validLabel;
}

Instruction pasm_stringToInstruction(char *line, ErrorCode *error, char **myLabel, char **unresolvedLabel) {
    Instruction instruction;

    /* Check for a valid label */
    if (myLabel) {
        char *labelEnd = strchr(line, ':');
        if (labelEnd) {
            if (validLabel(line, labelEnd - 1)) {
                *myLabel = (char*)malloc(labelEnd - line + 1);
                memcpy(*myLabel, line, labelEnd - line);
                (*myLabel)[labelEnd - line] = '\0';
                line = labelEnd + 1;
            }
        } else
            *myLabel = NULL;
        *unresolvedLabel = NULL;
    }

    char *opCode = strtok(line, " \t");
    char *operand = strtok(NULL, " \t\0");
    uppercase(opCode);

    *error = NO_ERROR;
    if (opCode == NULL) { /* There is no operator code */
        *error = UNSUPPORTED_OPCODE;
        instruction.opCode = NOP;
        instruction.addressMode = NO_ADDRESSING;
        instruction.operand = NO_OPERAND;
    /* Valid operator codes */
    } else if (strcmp(opCode, "NOP") == 0)
        instruction.opCode = NOP;
    else if (strcmp(opCode, "LDA") == 0)
        instruction.opCode = LDA;
    else if (strcmp(opCode, "LDB") == 0)
        instruction.opCode = LDB;
    else if (strcmp(opCode, "STA") == 0)
        instruction.opCode = STA;
    else if (strcmp(opCode, "STB") == 0)
        instruction.opCode = STB;
    else if (strcmp(opCode, "INP") == 0)
        instruction.opCode = INP;
    else if (strcmp(opCode, "OUT") == 0)
        instruction.opCode = OUT;
    else if (strcmp(opCode, "ADD") == 0)
        instruction.opCode = ADD;
    else if (strcmp(opCode, "SUB") == 0)
        instruction.opCode = SUB;
    else if (strcmp(opCode, "MUL") == 0)
        instruction.opCode = MUL;
    else if (strcmp(opCode, "DIV") == 0)
        instruction.opCode = DIV;
    else if (strcmp(opCode, "JMP") == 0)
        instruction.opCode = JMP;
    else if (strcmp(opCode, "JSP") == 0)
        instruction.opCode = JSP;
    else if (strcmp(opCode, "JSN") == 0)
        instruction.opCode = JSN;
    else if (strcmp(opCode, "JIZ") == 0)
        instruction.opCode = JIZ;
    else if (strcmp(opCode, "JOF") == 0)
        instruction.opCode = JOF;
    else if (strcmp(opCode, "JSB") == 0)
        instruction.opCode = JSB;
    else if (strcmp(opCode, "RTS") == 0)
        instruction.opCode = RTS;
    else if (strcmp(opCode, "HLT") == 0)
        instruction.opCode = HLT;
    else { /* Invalid operator code */
        *error = UNSUPPORTED_OPCODE;
        instruction.opCode = NOP;
        instruction.addressMode = NO_ADDRESSING;
        instruction.operand = NO_OPERAND;
    }

    /* The operator code is valid. Let's check the address mode and operand now. */
    if (*error == NO_ERROR) {
        if (operand == NULL) { /* There is no operand */
            instruction.addressMode = NO_ADDRESSING;
            instruction.operand = NO_OPERAND;
            if (needsOperand(instruction.opCode)) {
                instruction.opCode = NOP;
                *error = OPERAND_REQUIRED;
            }
        } else { /* There is an operand */
            if (operand[0] == '#' && (instruction.opCode == LDA || instruction.opCode == LDB)) {
                instruction.addressMode = IMMEDIATE_ADDRESSING;
                instruction.operand = atoi(operand + 1);
            } else if (operand[0] == '(' && operand[strlen(operand) - 1] == ')' &&
                       (instruction.opCode == LDA || instruction.opCode == LDB ||
                        instruction.opCode == STA || instruction.opCode == STB)) {
                instruction.addressMode = INDIRECT_ADDRESSING;
                if (operand[1] == '[' && operand[strlen(operand) - 2] == ']') {
                    if (validLabel(operand + 2, operand + strlen(operand) - 3)) {
                        if (unresolvedLabel) {
                            *unresolvedLabel = (char*)malloc(strlen(operand) - 3 /* - 4 + 1 */);
                            memcpy(*unresolvedLabel, operand + 2, strlen(operand) - 3);
                            (*unresolvedLabel)[strlen(operand) - 4] = '\0';
                        }
                        instruction.operand = 0; /* dummy operand */
                    } else {
                        instruction.opCode = NOP;
                        instruction.addressMode = NO_ADDRESSING;
                        instruction.operand = NO_OPERAND;
                    }
                } else
                    instruction.operand = atoi(operand + 1);
            } else if (operand[0] == '[' && operand[strlen(operand) - 1] == ']' && needsOperand(instruction.opCode)) {
                instruction.addressMode = DIRECT_ADDRESSING;
                if (validLabel(operand + 1, operand + strlen(operand) - 2)) {
                    if (unresolvedLabel) {
                        *unresolvedLabel = (char*)malloc(strlen(operand) - 1 /* - 2 + 1 */);
                        memcpy(*unresolvedLabel, operand + 1, strlen(operand) - 2);
                        (*unresolvedLabel)[strlen(operand) - 2] = '\0';
                    }
                    instruction.operand = 0; /* dummy operand */
                } else {
                    instruction.opCode = NOP;
                    instruction.addressMode = NO_ADDRESSING;
                    instruction.operand = NO_OPERAND;
                }
            } else if (needsOperand(instruction.opCode)) {
                if (operand[0] >= '0' && operand[0] <= '9') {
                    instruction.addressMode = DIRECT_ADDRESSING;
                    instruction.operand = atoi(operand);
                } else {
                    instruction.addressMode = NO_ADDRESSING;
                    *error = UNSUPPORTED_ADDRESSMODE;
                }
            }
        }
    }

    return instruction;
}

char *pasm_instructionToString(Instruction instruction) {
    char *str = NULL; /* Nullify pointer */
    int len = 0;
    /* Convert the operand to a signed value */
    int32_t operand = pasm_getSignedValue(instruction.operand);
    char beforeOperand = '\0', afterOperand = '\0';
    Boolean addOperand;

    if (instruction.addressMode == DIRECT_ADDRESSING) {
        len += 1;
    } else if (instruction.addressMode == IMMEDIATE_ADDRESSING) {
        len += 2;
        beforeOperand = '#';
    } else if (instruction.addressMode == INDIRECT_ADDRESSING) {
        len += 3;
        beforeOperand = '(';
        afterOperand = ')';
    }

    /* By using floor, log10 and abs we can calculate the amount of chars
       that have to be allocated for the numeric part of the operand to fit */
    len += floor(log10(abs(operand != 0 ? operand : 1))) + 1;
    if (operand < 0)
        len += 1;

    switch (instruction.opCode) {
        case NOP:
        case INP:
        case OUT:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case RTS:
        case HLT:
            str = (char*)malloc(4);
            addOperand = _FALSE;
            break;
        case LDA:
        case LDB:
        case STA:
        case STB:
        case JMP:
        case JSP:
        case JSN:
        case JIZ:
        case JOF:
        case JSB:
            str = (char*)malloc(len + 4);
            addOperand = _TRUE;
            break;
        default:
            return NULL;
    }

    if (str == NULL)
        pasm_error(HOST_MEMORY_EXHAUSTED);
    else {
        switch (instruction.opCode) {
            case NOP:
                strcpy(str, "NOP");
                break;
            case INP:
                strcpy(str, "INP");
                break;
            case OUT:
                strcpy(str, "OUT");
                break;
            case ADD:
                strcpy(str, "ADD");
                break;
            case SUB:
                strcpy(str, "SUB");
                break;
            case MUL:
                strcpy(str, "MUL");
                break;
            case DIV:
                strcpy(str, "DIV");
                break;
            case RTS:
                strcpy(str, "RTS");
                break;
            case HLT:
                strcpy(str, "HLT");
                break;
            case LDA:
                strcpy(str, "LDA ");
                break;
            case LDB:
                strcpy(str, "LDB ");
                break;
            case STA:
                strcpy(str, "STA ");
                break;
            case STB:
                strcpy(str, "STB ");
                break;
            case JMP:
                strcpy(str, "JMP ");
                break;
            case JSP:
                strcpy(str, "JSP ");
                break;
            case JSN:
                strcpy(str, "JSN ");
                break;
            case JIZ:
                strcpy(str, "JIZ ");
                break;
            case JOF:
                strcpy(str, "JOF ");
                break;
            case JSB:
                strcpy(str, "JSB ");
                break;
            default:
                return NULL;
        }

        if (addOperand) {
            if (beforeOperand == '\0') {
                sprintf(str + 4, "%d", operand);
            } else {
                str[4] = beforeOperand;
                sprintf(str + 5, "%d", operand);
            }
            if (afterOperand != '\0')
                str[len + 2] = afterOperand;
            str[len + 3] = '\0';
        }
    }
    return str;
}

uint32_t pasm_instructionToValue(Instruction instruction) {
    uint32_t value = 0;

    /* By using left shifts the correct value for this instruction can be created */
    value += instruction.opCode << 26;
    value += instruction.addressMode << 24;
    value += instruction.operand;

    return value;
}

Instruction pasm_valueToInstruction(int32_t value) {
    Instruction instruction;

    /* By using binary AND in combination with right shifting the operator code can be extracted */
    switch ((value & 0xfc000000) >> 26) {
        case NOP:
        case LDA:
        case LDB:
        case STA:
        case STB:
        case INP:
        case OUT:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case JMP:
        case JSP:
        case JSN:
        case JIZ:
        case JOF:
        case JSB:
        case RTS:
        case HLT:
            instruction.opCode = (value & 0xfc000000) >> 26;
            break;
        default:
            instruction.opCode = NOP;
    }

    /* By using binary AND in combination with right shifting the address mode can be extracted */
    switch ((value & 0x3000000) >> 24) {
        case IMMEDIATE_ADDRESSING:
        case DIRECT_ADDRESSING:
        case INDIRECT_ADDRESSING:
            instruction.addressMode = (value & 0x3000000) >> 24;
            break;
        default:
            instruction.addressMode = NO_ADDRESSING;
    }
    /* By using binary AND the operand can be extracted */
    instruction.operand = value & 0xffffff;

    return instruction;
}

int32_t pasm_getSignedValue(uint32_t unsignedValue) {
    int32_t signedValue;
    if (unsignedValue > MAX_SIGNED_POSITIVE_24BIT_NUMBER)
        signedValue = unsignedValue - MAX_UNSIGNED_24BIT_NUMBER - 1;
    else
        signedValue = unsignedValue;
    return signedValue;
}

uint32_t pasm_getUnsignedValue(int32_t signedValue) {
    uint32_t unsignedValue;
    if (signedValue < 0)
        unsignedValue = MAX_UNSIGNED_24BIT_NUMBER + signedValue + 1;
    else
        unsignedValue = signedValue;
    return unsignedValue;
}

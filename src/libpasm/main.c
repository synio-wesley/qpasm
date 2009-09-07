#include "pasm.h"
#include <stdio.h>

void registerHasChanged() {
    const Register reg = pasm_getRegister();
    printf("Register has changed!\n");
    printf("A: %d\n", reg.a);
    printf("B: %d\n", reg.b);
    printf("PC: %d\n", reg.programCounter);
}

void readInput() {
    int input;
    printf("Number input: ");
    scanf("%d", &input);
    pasm_setInput(input);
}

void stop() {
    printf("-- Program reports to have ended.\n");
}

int main() {
    printf("Testing!\n");
    pasm_init();
    pasm_registerCallback(REGISTER_CHANGE, registerHasChanged);
    /*pasm_registerCallback(ERROR_MESSAGE, errorMsg);*/
    pasm_registerCallback(READ_INPUT, readInput);
    /*pasm_registerCallback(OUTPUT_AVAILABLE, output);*/
    pasm_registerCallback(PROGRAM_END, stop);

    pasm_compileProgram("INP\nSTA 10\nJSN 4\nJMP 6\nLDB #-1\nMUL\nSTA 11\nOUT\nHLT", 5);
    pasm_runProgram();
    pasm_reset();
    pasm_compileProgram("INP\nSTA 10\nJSN 4\nJMP 6\nLDB #-1\nMUL\nSTA 11\nOUT\nHLT", 20);
    pasm_runProgram();
    pasm_free();

    return 0;
}

# Installation #

We do not provide an installer, but instead provide archives containing the executable and the required dependencies. Just unpack the archive and run QPasm. The configuration file will be saved in the same directory as the executable in a portable format.

# Configuring QPasm #

The layout and geometry of the QPasm window is saved automatically. If you would like to reset the layout to the default layout, use the _Restore default layout_ option which you can find by opening the Settings window by clicking on _Edit -> Settings_.

You can also tweak the timestep interval, and the font and colors for the text editor in the Settings window.

# Writing and running PASM applications #

You can write pseudo-assembler applications in the _Code Editor_.

## Supported instructions ##

| **Command** | **What it does** |
|:------------|:-----------------|
| NOP | Does nothing |
| LDA #X, LDB #X | Load value X in register A or B |
| LDA X, LDB X | Load contents of address X in register A or B |
| LDA (X), LDB (X) | Load contents of address pointed to by address X in register A or B |
| STA X, STB X | Store contents of register A or B in address X |
| STA (X), STB (X) | Store contents of register A or B in address pointed to by address X |
| INP | Ask for input from the user and store inputted value in register A |
| OUT | Output contents of register A |
| ADD | Add contents of register A to contents of register B and store the result in register A |
| SUB | Subtract contents of register B from contents of register A and store the result in register A |
| MUL | Multiply contents of register A by contents of register B and store the result in register A |
| DIV | Divide contents of register A by contents of register B and store the result in register A |
| JMP X | Unconditional jump to instruction at address X |
| JSP X | Jump to instruction at address X if contents of register A is strictly positive |
| JSN X | Jump to instruction at address X if contents of register A is strictly negative |
| JIZ X | Jump to instruction at address X if contents of register A is equal to 0 |
| JOF X | Jump to instruction at address X if the previous mathematical instruction caused an overflow |
| JSB X | Unconditional jump to subroutine at address X |
| RTS | Return from subroutine |
| HLT | End of assembler program |

## Using labels ##

Because working with addresses can be very confusing, QPasm supports labels. You can attach labels to addresses by prepending the instruction in the _Code Editor_ with a variable which may contain lowercase and uppercase letters and the underscore symbol. You can refer to labels by putting a label between square brackets instead of providing an address. An example of a simple program using labels follows:

```
ask_number: INP
            JIZ [if_zero]
            JMP [ask_number]
if_zero:    HLT
```

This sample program will keep asking the user for numbers until the inputted number is equal to zero.

## Using variables ##

Just use NOP instructions with labels to create static variables. An example:

```
var: NOP
     LDA #8
     STA [var]
```

_var_ will now contain the value 8.

## Using comments ##

You can write comments on a seperate line or after instructions. QPasm automatically knows the difference between instructions and comments. If a comment is placed on a seperate line, the comment will be compiled into a NOP instruction.

## About whitespace ##

Spaced and tabs may occur before and after instructions, as well as between the instruction operator and its operand.

## Running your application ##

Just press the _Compile_ button or press _Ctrl+F7_ to compile your application and press the green _Run_ button or press _Ctrl+F5_ to test your program.

# Debugging PASM applications #

QPasm has some integrated debugging capabilities.

## Stepping through your program ##

By clicking the blue _Step_ button or pressing _F5_ you can execute instructions in your program one by one. The instruction that will be executed next will be highlighted in a blue color in the _Code Editor_ and a red dot will indicate it in the _Memory_ table.

You can also use the _Timed step_ button to execute instructions one by one with a specified interval in between them. The interval can be configured in the _Settings_ window by clicking on _Edit -> Settings_. You can use the _Pause_ button or press _CTRL+P_ to pause the application.

After the program has been paused you can edit data on-the-fly (see below) or you can resume the program by clicking on either the _Step_, _Timed Step_ or _Run_ button. You can use the _Stop_ button to stop execution of the application manually.

## Using breakpoints ##

If you would like to pause your program automatically before a certain instruction is executed, you can set a breakpoint on that line. To do so, click in front of the line you want to add a breakpoint for (or click inside the line and press the + button under the list of breakpoints). You can remove breakpoints in the same way.

You have to check _Enable breakpoints_ to enable breakpoints as well. After some breakpoints have been set and if breakpoints are enabled, you can run your application. Your application will now pause automatically when a breakpoint is reached and the line with the breakpoint will be marked red.

After the program has been paused you can edit data on-the-fly (see below) or you can resume the program by clicking on either the _Step_, _Timed Step_ or _Run_ button. You can use the _Stop_ button to stop execution of the application manually.

## Editing data on-the-fly ##

You can add or edit memory addresses when the application has not started yet or has been paused. When editing instructions, QPasm will automatically resolve labels on-the-fly when needed. You can edit the program counter, registers and flags when the application has been paused.
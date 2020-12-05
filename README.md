Attention this in a "WORK IN PROGRESS" 6502 library. More documentation
to follow

INTRODUCTION

I have a working 6502 emulator written in C under LINUX (I am using MINT, but any
distro should be able to compile). 

I have written a small test program that loads the famous Klaus2m5 functional tests
in memory and then runs it. It completes successfully in around 1s on my computer. 

For now, this library only emulates original documented 6502 code. It does not emulate 
(yet) 65C02 opcodes. Also, non-documented opcodes are treated as NOP. Cycle counting 
is not yet perfect, as we don't detect cases where operands spill out of a page (which
should consume an extra cycle for some commands). 

The Makefile will compile and create a static library, and then it will compile 
and link the test program. In order to run it, you need to place the binary test 
file in the same directory as the program. It can be obtained here: 

https://github.com/Klaus2m5/6502_65C02_functional_tests/tree/master/bin_files

You only need file 6502_functional_test.bin

Carefull with make clean, as it will delete all your .o files in the same 
directory (you should be ok if you put my files on an empty directory). 

GLOBAL VARIABLES

This library define a struct to be used by a single global variable that needs to 
be defined in the user program. The use of this global variables is important for 
speed, as it reduces the need of stack parameter passing between the user code 
and the library functions. 

The global variable is of type microprocessor, which is a structure that contains
the microprocessor registers of type unsigned char (unsigned short for PC) and 
also an unsigned long used to count the number of cyles executed.

The global variable must be defined in used code as: 

microprocessor cpu;


LIBRARY FUNCTIONS 

The library has only 2 externally accessible functions: 

int processcommand();

This function takes no parameters. It will basically read the opcode pointed by
the current value of the PC register on the CPU and execute it, reading the 
operands, updating the status register flag, the PC register and also adding the
cycles taken by the command. 

For now it always return a zero. 

  
unsigned char fetchmemory();

This function takes no parameters. It returns the value pointed by the current
amount at the PC register. It will then increase the PC register automatically. 


EXTERNAL FUNCTIONS

In addition to the global variable cpu described above, the library contains two
user defined external functions that need to be implemented on the user code. 

The serve as an interface between the cpu bus and the external addressable 
devices, usually RAM, ROM and I/O devices. 

The example program distributed with the library implements these external
addressable devices as a 64K array of 8-bit unsigned char and implements 
these functions to return and update the contents of the array directly.

On a real system, some logic must be implemented so that read and write
affect the correct external device according to the address they are 
accessing. 



To use my library on your own code, you need to: 

1) Include 6502.h in your source code
2) Create a readmemory and writememory function in your code
    The emulator will call those functions when it need to read/write from the bus
3) Create cpu variable of type microprocessor
4) Initialize the CPU registers
    -   Set status register to 0x20
    -   Set other registers to 0
    -   Setup the cpu.pc to the starting memory address of your program
5) call processcommand in a loop, to execute program
6) You may set #define DEBUG 1 to generate debug information
    -   Careful, this will fill stderr with one line for each opcode processed

Please refer to test6502.c for a source code example of how the library currently
works. 

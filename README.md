Attention this is a "WORK IN PROGRESS" 6502 library. More documentation
to follow


WHAT'S NEW 

The library will now emulate most of the undocummented opcodes, though the code has
not yet been tested. Some unstable codes are still not emulated. 

Minor fixes on brk / rti code. Removed duplicate setting of interrupt flag on brk, 
and fixed mask for clearing bits 4 and 5 of pulled status on rti. 

Removed typdef from cpu struct definition as it was not needed. 

Improved decimal mode operation: Non-valid BCD emulated, implemented behavior of
all flags. Emulator now successfully runs 6502_decimal_test. Test program added
(you still need to download the new test file from Klaus site below)


INTRODUCTION

I have a working 6502 emulator written in C under LINUX (I am using MINT, but any
distro should be able to compile). 

I have written a small test program that loads the famous Klaus2m5 functional tests
in memory and then runs it. It completes successfully in around 1s on my computer. 

The library will now emulate most of the undocummented opcodes, though the code has
not yet been tested. Some unstable codes are still not emulated. This library does 
not emulate (yet) 65C02 opcodes. 

The Makefile will compile and create a static library, and then it will compile 
and link the test program. In order to run it, you need to place the binary test 
file in the same directory as the program. It can be obtained here: 

https://github.com/Klaus2m5/6502_65C02_functional_tests/tree/master/bin_files

You only need file 6502_functional_test.bin

Carefull with make clean, as it will delete all your .o files in the same 
directory (you should be ok if you put my files on an empty directory). 


GLOBAL VARIABLES

This library define a struct to be used by a single global variable called cpu of
type microprocessor, which is a struct (defined in 6502.h) which contains all the
registers of the cpu and it can be accessed by the user code. 
The use of this global variable is important for speed, as it reduces the need of
stack parameter passing between the user code and the library functions. 


EXTERNAL FUNCTIONS

In addition to the global variable cpu described above, the library contains two
user defined external functions that need to be implemented on the user code. 

They serve as an interface between the cpu bus and the external addressable 
devices, usually RAM, ROM and I/O devices. 

The example program distributed with the library implements these external
addressable devices as a 64K array of 8-bit unsigned char and implements 
these functions to return and update the contents of the array directly.

On a real system, some logic must be implemented so that read and write
affect the correct external device according to the address they are 
accessing. 


LIBRARY FUNCTIONS 

The library now has three externally accessible functions: 

int processcommand();

This function takes no parameters. It will basically read the opcode pointed by
the current value of the PC register on the CPU and execute it, reading the 
operands, updating the status register flag, the PC register and also adding the
cycles taken by the command. 

For now it always return a zero. 

void interrupt();

This function generates a HW interrupt if the interrupt flag on the status
register is not set. It will push the current program counter and the status
register into the stack and then execute the opcode in the  address pointed by
$FFFE/$FFFF

void nmi();

This function generates a non-maskable interrupt independent of the value
of the interrupt flag in the status register. It will push the current 
program counter and the status register into the stack and then execute
the opcode in the address pointed by $FFFA/$FFFB
  

To use my library on your own code, you need to: 

1) Include 6502.h in your source code
2) Create a readmemory and writememory function in your code
    The emulator will call those functions when it need to read/write from the bus
4) Initialize the CPU registers
    -   Set status register to 0x20
    -   Set other registers to 0
    -   Setup the cpu.pc to the starting memory address of your program
5) call processcommand in a loop, to execute program
6) You may set #define DEBUG 1 in 6502.h to generate debug information
    -   Careful, this will fill stderr with one line for each opcode processed

Please refer to test6502.c for a source code example of how the library currently
works. 

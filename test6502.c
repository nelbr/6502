// 
// 6502 emulator written in C
//
// An education project for me to learn about 6502 emulation
//
// Maybe a long term goal of extending this into an apple 2 emulator
//
// This test program will run a set of tests I downloaded from the
// Internet from Klaus2m5. It needs the binary file called 
// 6502_functional_test.bin which you can download from: 
//
// https://github.com/Klaus2m5/6502_65C02_functional_tests/tree/master/bin_files
//
// The test never finishes, so there is probably still some emulation
// bug in my code. Please let me know if you find anything.
//
// nelbr - June/July 2020
//
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "6502.h"

microprocessor cpu;
unsigned char memory[65536];
int  i;
int used=0;
int sound=0;

//
// Read binary file in memory
//
void rominit()
{
    FILE *fp;
    int result;
    printf ("Reading memory file ./6502_functional_test.bin\n");
    fp = fopen ( "6502_functional_test.bin", "r" );
    result = fread (&memory,1,65536,fp);
    fclose(fp);
    printf ("file size read %d\n", result);
}
       
//
// Initialize 6502 processor registers. The test program code 
// starts at address 0x0400
//
void boot()
{
    cpu.a = 0x00;
    cpu.x = 0x00;
    cpu.y = 0x00;
    cpu.sp= 0xFF;
    cpu.pc= 0x0400;
    cpu.status= 0x20;
}
//
// Readmemory routine in this example just returns value of 64K array
//
unsigned char readmemory(unsigned short address)
{
    unsigned char result;
    result = memory[address];
    return result;
}

//
// Writememory routine in this example just sets value of 64K array
//
void writememory(unsigned short address, unsigned char value)
{
    memory[address] = value;
}

//
// Main function of test routine
//
int main()
{
	rominit();
    boot();
    while (processcommand()==0) 
    {
        // 
        // We can set debug to trace command execution on stderr. 
        // Careful though, this will generate a substantial amount of output
        //
        #ifdef DEBUG
        if (used) fprintf (stderr, " A=%02X, X=%02X, Y=%02X, SP=%02X, PC=%02X, STATUS=%02X", cpu.a, cpu.x, cpu.y, cpu.sp, cpu.pc, cpu.status); 
        else fprintf (stderr, "      A=%02X, X=%02X, Y=%02X, SP=%02X, PC=%02X, STATUS=%02X", cpu.a, cpu.x, cpu.y, cpu.sp, cpu.pc, cpu.status); 
        fprintf(stderr, STATUS_TO_BINARY_PATTERN, STATUS_TO_BINARY(cpu.status));
        #endif 

        printf ("Teste numero %2X %2X %2X\n", memory[0x200], memory[0x203], memory[0x204]);
        used=0;
    }
    printf ("BREAK A=%02X, X=%02X, Y=%02X, SP=%02X, PC=%04X, STATUS=%02X\n", cpu.a, cpu.x, cpu.y, cpu.sp, cpu.pc, cpu.status); 
    return 0;
}
	
        
/*
Pseudo code:
1- read ROM into memory
2- run boot routine to initialize registers
3- read command from memory pointed by pc
4- read command parameters (0, 1, 2...)
5- process command
6- update pc
7- goto 3
*/

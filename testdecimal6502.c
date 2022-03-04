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
// nelbr - June/July 2020
//
#include <stdio.h>
#include <sys/time.h>
#include "6502.h"

unsigned char memory[65536];

//
// Read binary file in memory
//
int rominit()
{
    FILE *fp;
    int result;
    printf ("Reading memory file ./6502_decimal_test.bin\n");
    fp = fopen ( "6502_decimal_test.bin", "r" );
    if ( fp == NULL ) return 8;
    result = fread (&memory[0x200],1,258,fp);
    fclose(fp);
    printf ("file size read %d\n", result);
    return 0;
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
    cpu.pc= 0x0200;
    cpu.status= 0x20;
    cpu.cycles= 0;
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
    struct timeval start,stop;
    long seconds, micros; 

    // 
    // Load test code into memory, exit program if file does can't be loaded
    //
	if (rominit()!=0) {
        printf( "Could not open binary test file\n" ) ;
        printf( "This program requires the file 6502_decimal_test.bin (see README for link to download)\n");
        return 0;
    }

    //
    // Initialize cpu registers
    //
    boot();
    printf ("Running test, please wait a bit\n");

    //
    // Record start time
    //
    gettimeofday(&start, NULL);

    //
    // Main loop, sequentially execute commands pointed by the program counter
    // register in the CPU. 
    //
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
        used=0;
        #endif 

        if (cpu.pc<0x200) break;
        // printf ("PC=%4X Op=%2X A=%2X X=%2X Y=%2X P=%2X, DesiredP=%2X N1=%2X, N2=%2X DA=%2X, AR=%2X DNVZC=%2X VF=%2X\n", cpu.pc, memory[cpu.pc], cpu.a, cpu.x, cpu.y, cpu.status, memory[0x0005], memory[0x0000], memory[0x0001], memory[0x0004], memory[0x0006], memory[0x0005], memory[0x0008]); 

    }

    // 
    // Record test completion time and calculate time spent in micro seconds
    //
    gettimeofday(&stop, NULL);
    seconds = (stop.tv_sec - start.tv_sec);
    micros = (seconds * 1000000) + stop.tv_usec - start.tv_usec;

    //
    // Output results
    //
    printf ("Number of cycles spent = %ld\n", cpu.cycles);
    if (!memory[0x000B]) printf ("Test completed successfully in %ld us\n",micros);
    else                printf ("Test has FAILED in %ld us\n",micros);
    printf ("Estimated CPU speed in this computer = %ld Mhz\n", (cpu.cycles/micros));
    // printf ("BREAK A=%02X, X=%02X, Y=%02X, SP=%02X, PC=%04X, STATUS=%02X\n", cpu.a, cpu.x, cpu.y, cpu.sp, cpu.pc, cpu.status); 
    
    //
    // End we are done
    //
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

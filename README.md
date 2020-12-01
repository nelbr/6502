Attention this in a "non working" WORK IN PROGRESS 6502 library. More documentation
to follow

I have a working 6502 emulator written in C under LINUX (I am using MINT, but any
distro should be able to compile). 

I have written a small test program that loads the famous Klaus2m5 functional tests
in memory and then runs it. It completes successfully in around 1s on my computer. 

The Makefile will compile and create a static library, and then it will compile 
and link the test program. In order to run it, you need to place the binary test 
file in the same directory as the program. It can be obtained here: 

https://github.com/Klaus2m5/6502_65C02_functional_tests/tree/master/bin_files

You only need file 6502_functional_test.bin

Carefull with make clean, as it will delete all your .o files in the same 
directory (you should be ok if you put my files on an empty directory). 

To use my library on your own code, you need to: 

1) Include 6502.h in your source code
2) Create a readmemory and writememory function in your code
    The emulator will call those functions when it need to read/write from the bus
3) Create cpu variable of type microprocessor
4) Setup the cpu.pc to the starting memory address of your program
5) call processcommand in a loop, to execute program

Please refer to test6502.c for a source code example of how the library currently
works. 

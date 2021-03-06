// 
// 6502 emulator written in C
//
// An education project for me to learn about 6502 emulation
//
// Maybe a long term goal of extending this into an apple 2 emulator
//
//    This file is part of lib6502.
//
//    lib6502 is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    lib6502 is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
//
// nelbr - Summer 2020
//

#include <stdio.h>
#include "6502.h"

#define IMMEDIATE 1
#define ZERO_PAGE 2
#define ZERO_PAGE_X 3
#define ZERO_PAGE_Y 4
#define ABSOLUTE 5
#define ABSOLUTE_X 6
#define ABSOLUTE_Y 7
#define INDIRECT_X 8
#define INDIRECT_Y 9
#define IMPLIED 10
#define ACCUMULATOR 11
#define INDIRECT 12
#define RELATIVE 13

unsigned char bordercross;

// 
// Read the next opcode from current pc value
//
__attribute((always_inline)) inline unsigned char fetchmemory()
{
    unsigned char result;
    result = readmemory(cpu.pc);
    if (cpu.pc<0xFFFF) cpu.pc++;
    else cpu.pc=0;
    return result;
} 

//
// Return address referenced by the addressing mode
//
__attribute((always_inline)) inline unsigned short get_address(unsigned char mode)
{
    unsigned char operand;
    unsigned char operand_l;
    unsigned char operand_h;
    unsigned short address;
    switch (mode) {
	case ZERO_PAGE:
	    operand = fetchmemory();
	    address = (unsigned short) operand;
	    break; 

	case ZERO_PAGE_X:
	    operand = fetchmemory();
	    address = (unsigned short) operand + cpu.x;
	    if (address>0xFF) address = address - 0x100;
	    break; 

	case ZERO_PAGE_Y:
	    operand = fetchmemory();
	    address = (unsigned short) operand + cpu.y;
	    if (address>0xFF) address = address - 0x100;
	    break; 

	case ABSOLUTE:
	    operand_l = fetchmemory();
	    operand_h = fetchmemory();
	    address = (unsigned short) ( operand_h << 8 | operand_l );
	    break; 

	case ABSOLUTE_X:
	    operand_l = fetchmemory();
	    operand_h = fetchmemory();
	    address = (unsigned short) ( operand_h << 8 | operand_l ) + cpu.x;
        if (((address & 0xFF00)>>8) != operand_h) bordercross=1; 
	    break; 

	case ABSOLUTE_Y:
	    operand_l = fetchmemory();
	    operand_h = fetchmemory();
	    address = (unsigned short) ( operand_h << 8 | operand_l ) + cpu.y;
        if (((address & 0xFF00)>>8) != operand_h) bordercross=1; 
	    break; 

    case INDIRECT:
        operand_l = fetchmemory();
        operand_h = fetchmemory();
	    address = (unsigned short) ( operand_h << 8 | operand_l );
        // please note that the 6502 has a bug that causes it to take operand_h below
        // from the same page if operand_l is on position 0xFF of the page. The 65C02
        // fixes this bug. The implementation below follows the 6502 behaviour.
        // 
        // Note: The bug only occurs with the jmp opcode. 
        if (operand_l == 0xFF) operand_h = readmemory(address-255);
        else operand_h = readmemory(address+1);
        operand_l = readmemory(address);
	    address = (unsigned short) ( operand_h << 8 | operand_l );
        break;

	case INDIRECT_X:
	    operand = fetchmemory();
	    address = (unsigned short) operand + cpu.x;
	    if (address>0xFF) address = address - 0x100;
	    operand_l = readmemory(address);
	    if (address<0xFF) operand_h = readmemory(address+1);
	    else operand_h = readmemory(0x0000);
	    address = (unsigned short) ( operand_h << 8 | operand_l );
	    break; 

	case INDIRECT_Y:
	    operand = fetchmemory();
	    address = (unsigned short) operand;
	    operand_l = readmemory(address);
	    if (address<0xFF) operand_h = readmemory(address+1);
	    else operand_h = readmemory(0x0000);
	    address = (unsigned short) ( operand_h << 8 | operand_l ) + cpu.y;
        if (((address & 0xFF00)>>8) != operand_h) bordercross=1; 
	    break; 
	}
#ifdef DEBUG
    fprintf (stderr, "%04X ", address);
#endif 
    used=1;
	return address;
}

__attribute((always_inline)) inline void adc (unsigned char mode) 
{
    unsigned short sum; 
    unsigned short suml, sumh;
    unsigned char al, ah, ol, oh;
    unsigned char operand;
#ifdef DEBUG
    fprintf(stderr,"adc ");
#endif
    if (mode==IMMEDIATE) operand = fetchmemory();
    else operand = readmemory(get_address(mode));

    if ((cpu.status & 1UL<<3)>>3) { 
        al = cpu.a & 0x0F;
        ol = operand & 0x0F;
        ah = cpu.a & 0xF0;
        oh = operand & 0xF0;
        if (cpu.status & 1UL<<0) suml = al + ol + 1; 
        else                     suml = al + ol;
        sumh = ah + oh;
        if (suml>=0x0A) {
            suml -= 0x0A;
            sumh += 0x10;
        }
        if (sumh>=0xA0) {
            sumh -= 0xA0;
            cpu.status |= 1UL << 0;     // set bit carry on status processor 
        }
        else {
            cpu.status &= ~(1UL << 0);  // clear bit carry on status processor
        }
        sum = sumh + suml;
        cpu.a = (char) sum; 
    }
    else {
        if (cpu.status & 1UL<<0) sum = cpu.a + operand + 1; 
        else                     sum = cpu.a + operand;
        if (sum>0xFF)    cpu.status |= 1UL << 0; else cpu.status &= ~(1UL << 0);  // set bit carry on status processor
        if ((!((cpu.a ^ operand) & 0x80) && ((cpu.a ^ sum) & 0x80))!=0) cpu.status |= 1UL << 6; else cpu.status &= ~(1UL << 6); // set bit overflow   
        cpu.a = (char) sum; 
        if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
        if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
    }

}

__attribute((always_inline)) inline void fand (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"and ");
#endif 
    if (mode==IMMEDIATE) cpu.a = cpu.a & fetchmemory();
    else cpu.a = cpu.a & readmemory(get_address(mode));

    if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}
    
__attribute((always_inline)) inline void asl (unsigned char mode) 
{
    unsigned short aux;
    unsigned short val;
#ifdef DEBUG
    fprintf(stderr,"asl ");
#endif 
    if (mode==ACCUMULATOR)
    {
        if (cpu.a>=0x80) cpu.status |= 1UL << 0; else cpu.status &= ~(1UL << 0); // set bit carry on status processor
        cpu.a &= ~(1UL << 7);                                                    // set bit 7 of accumulator to 0
        cpu.a = cpu.a << 1;       
        if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
        if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
    }
    else
    {
        aux = get_address(mode);
        val = readmemory(aux);
        if (val>=0x80) cpu.status |= 1UL << 0; else cpu.status &= ~(1UL << 0); // set bit carry on status processor to true
        val &= ~(1UL << 7);                                                    // set bit 7 of input to 0
        val = val << 1;
        writememory ( aux, val );
        if (!val)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1); // set bit zero on status processor to true
        if (val>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7); // set bit negative on status processor to true
    }
}

__attribute((always_inline)) inline void bcc (unsigned char mode) 
{
    unsigned char branch;
    unsigned short currpage;
#ifdef DEBUG
    fprintf(stderr,"bcc ");
#endif 
    branch= fetchmemory();
    currpage = (cpu.pc & 0xFF00);
    if (!(cpu.status & (1UL << 0)))
    {
       if (branch>=0x80) cpu.pc -= (0x100 - branch);
       else              cpu.pc += branch;
       cpu.cycles += 1;
    }
    if ((cpu.pc & 0xFF00) != currpage) cpu.cycles += 1;
}

__attribute((always_inline)) inline void bcs (unsigned char mode) 
{
    unsigned char branch;
    unsigned short currpage;
#ifdef DEBUG
    fprintf(stderr,"bcs ");
#endif 
    branch=fetchmemory();
    currpage = (cpu.pc & 0xFF00);
    if (cpu.status & (1UL << 0))
    {
       if (branch>=0x80) cpu.pc -= (0x100 - branch);
       else              cpu.pc += branch;
       cpu.cycles += 1;
    }
    if ((cpu.pc & 0xFF00) != currpage) cpu.cycles += 1;
}

__attribute((always_inline)) inline void beq (unsigned char mode) 
{
    unsigned char branch;
    unsigned short currpage;
#ifdef DEBUG
    fprintf(stderr,"beq ");
#endif 
    branch=fetchmemory();
    currpage = (cpu.pc & 0xFF00);
    if (cpu.status & (1UL << 1))
    {
       if (branch>=0x80) cpu.pc -= (0x100 - branch);
       else              cpu.pc += branch;
       cpu.cycles += 1;
    }
    if ((cpu.pc & 0xFF00) != currpage) cpu.cycles += 1;
}

__attribute((always_inline)) inline void bit (unsigned char mode) 
{
    unsigned short aux;
    unsigned char val;
#ifdef DEBUG
    fprintf(stderr,"bit ");
#endif 
    aux = get_address(mode);
    val = readmemory(aux);
    if (!(val & cpu.a)) cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1); // set bit zero on status processor
    cpu.status = ((cpu.status & ~(1UL << 6)) | (val & 1UL << 6)); // set bit overflow on status processor to 6th bit of memory
    cpu.status = ((cpu.status & ~(1UL << 7)) | (val & 1UL << 7)); // set bit negative on status processor to 7th bit of memory
}

__attribute((always_inline)) inline void bmi (unsigned char mode) 
{
    unsigned char branch;
    unsigned short currpage;
#ifdef DEBUG
    fprintf(stderr,"bmi ");
#endif 
    branch=fetchmemory();
    currpage = (cpu.pc & 0xFF00);
    if (cpu.status & (1UL << 7))
    {
       if (branch>=0x80) cpu.pc -= (0x100 - branch);
       else              cpu.pc += branch;
       cpu.cycles += 1;
    }
    if ((cpu.pc & 0xFF00) != currpage) cpu.cycles += 1;
}

__attribute((always_inline)) inline void bne (unsigned char mode) 
{
    unsigned char branch;
    unsigned short currpage;
#ifdef DEBUG
    fprintf(stderr,"bne ");
#endif 
    branch=fetchmemory();
    currpage = (cpu.pc & 0xFF00);
    if (!(cpu.status & (1UL << 1)))
    {
       if (branch>=0x80) cpu.pc -= (0x100 - branch);
       else              cpu.pc += branch;
       cpu.cycles += 1;
    }
    if ((cpu.pc & 0xFF00) != currpage) cpu.cycles += 1;
}

__attribute((always_inline)) inline void bpl (unsigned char mode) 
{
    unsigned char branch;
    unsigned short currpage;
#ifdef DEBUG
    fprintf(stderr,"bpl ");
#endif 
    branch=fetchmemory();
    currpage = (cpu.pc & 0xFF00);
    if (!(cpu.status & (1UL << 7)))
    {
       if (branch>=0x80) cpu.pc -= (0x100 - branch);
       else              cpu.pc += branch;
       cpu.cycles += 1;
    }
    if ((cpu.pc & 0xFF00) != currpage) cpu.cycles += 1;
}

__attribute((always_inline)) inline void fbrk (unsigned char mode)
{
    unsigned char operand_l, operand_h;
#ifdef DEBUG
    fprintf(stderr,"brk ");
#endif 
    operand_l = (char) (cpu.pc+1);
    operand_h = (char) ((cpu.pc+1)>>8);
    writememory(0x100+cpu.sp, operand_h);
    cpu.sp--;
    writememory(0x100+cpu.sp, operand_l);
    cpu.sp--;
    writememory(0x100+cpu.sp, cpu.status | 0x30);  // set bits break and reserved to true on the stack copy of the status register
    cpu.sp--;
    cpu.status |= 0x04;
    operand_l = readmemory(0xFFFE);
    operand_h = readmemory(0xFFFF);
    cpu.pc = (operand_h << 8) + operand_l;
    cpu.status |= 0x04;
}


__attribute((always_inline)) inline void bvc (unsigned char mode) 
{
    unsigned char branch;
    unsigned short currpage;
#ifdef DEBUG
    fprintf(stderr,"bvc ");
#endif 
    branch=fetchmemory();
    currpage = (cpu.pc & 0xFF00);
    if (!(cpu.status & (1UL << 6)))
    {
       if (branch>=0x80) cpu.pc -= (0x100 - branch);
       else              cpu.pc += branch;
       cpu.cycles += 1;
    }
    if ((cpu.pc & 0xFF00) != currpage) cpu.cycles += 1;
}

__attribute((always_inline)) inline void bvs (unsigned char mode) 
{
    unsigned char branch;
    unsigned short currpage;
#ifdef DEBUG
    fprintf(stderr,"bvs ");
#endif 
    branch=fetchmemory();
    currpage = (cpu.pc & 0xFF00);
    if ((cpu.status & (1UL << 6)))
    {
       if (branch>=0x80) cpu.pc -= (0x100 - branch);
       else              cpu.pc += branch;
       cpu.cycles += 1;
    }
    if ((cpu.pc & 0xFF00) != currpage) cpu.cycles += 1;
}

__attribute((always_inline)) inline void clc (unsigned char mode)
{
#ifdef DEBUG
    fprintf(stderr,"clc ");
#endif 
    cpu.status &= ~(1UL << 0);     // clear bit carry on status processor to true
}

__attribute((always_inline)) inline void cld (unsigned char mode)
{
#ifdef DEBUG
    fprintf(stderr,"cld ");
#endif 
    cpu.status &= ~(1UL << 3);     // clear bit decimal on status processor to true
}

__attribute((always_inline)) inline void cli (unsigned char mode)
{
#ifdef DEBUG
    fprintf(stderr,"cli ");
#endif 
    cpu.status &= ~(1UL << 2);     // clear bit interrupt on status processor to true (interrupt disabled)
}

__attribute((always_inline)) inline void clv (unsigned char mode)
{
#ifdef DEBUG
    fprintf(stderr,"clv ");
#endif 
    cpu.status &= ~(1UL << 6);     // clear bit overflow on status processor to true (interrupt disabled)
}

__attribute((always_inline)) inline void cmp (unsigned char mode) 
{
    unsigned char tmp;
#ifdef DEBUG
    fprintf(stderr,"cmp ");
#endif 
    if (mode==IMMEDIATE) tmp = fetchmemory(); 
    else tmp = readmemory(get_address(mode));

    if (cpu.a >= tmp) cpu.status |= 1UL << 0; else cpu.status &= ~(1UL << 0);               // set bit carry on status processor to true
    if (cpu.a == tmp) cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);               // set bit zero on status processor to true
    if ((cpu.a - tmp) & (1UL << 7)) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7); // set bit negative on status processor to true
}
    
__attribute((always_inline)) inline void cpx (unsigned char mode) 
{
    unsigned char tmp;
#ifdef DEBUG
    fprintf(stderr,"cpx ");
#endif 
    if (mode==IMMEDIATE) tmp = fetchmemory();
    else tmp = readmemory(get_address(mode));

    if (cpu.x >= tmp) cpu.status |= 1UL << 0; else cpu.status &= ~(1UL << 0);               // set bit carry on status processor to true
    if (cpu.x == tmp) cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);               // set bit zero on status processor to true
    if ((cpu.x - tmp) & (1UL << 7)) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7); // set bit negative on status processor to true
}

__attribute((always_inline)) inline void cpy (unsigned char mode) 
{
    unsigned char tmp;
#ifdef DEBUG
    fprintf(stderr,"cpy ");
#endif 
    if (mode==IMMEDIATE) tmp = fetchmemory();
    else tmp = readmemory(get_address(mode));

    if (cpu.y >= tmp) cpu.status |= 1UL << 0; else cpu.status &= ~(1UL << 0);               // set bit carry on status processor to true
    if (cpu.y == tmp) cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);               // set bit zero on status processor to true
    if ((cpu.y - tmp) & (1UL << 7)) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7); // set bit negative on status processor to true
}

__attribute((always_inline)) inline void dec (unsigned char mode) 
{
    unsigned short aux;
    unsigned short val;
#ifdef DEBUG
    fprintf(stderr,"dec ");
#endif 
    aux = get_address(mode);
    val = readmemory(aux);
    val--;
    writememory(aux, val);

    if (!val)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (val>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void dex (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"dex ");
#endif 
    if (cpu.x!=0x00) cpu.x--; 
    else cpu.x=0xFF;
    if (!cpu.x)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.x>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void dey (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"dey ");
#endif 
    if (cpu.y!=0x00) cpu.y--; 
    else cpu.y=0xFF;
    if (!cpu.y)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.y>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void eor (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"eor ");
#endif 
    if (mode==IMMEDIATE) cpu.a = cpu.a ^ fetchmemory();
    else cpu.a = cpu.a ^ readmemory(get_address(mode));

    if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void inc (unsigned char mode) 
{
    unsigned short aux;
    unsigned short val;
#ifdef DEBUG
    fprintf(stderr,"inc ");
#endif 
    aux = get_address(mode);
    val = readmemory(aux);
    if (val!=0xFF) val++;
    else val=0;
    writememory(aux, val);

    if (!val)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1); // set bit zero on status processor to true
    if (val>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7); // set bit negative on status processor to true
}

__attribute((always_inline)) inline void inx (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"inx ");
#endif 
    if (cpu.x!=0xFF) cpu.x++; 
    else cpu.x=0;

    if (!cpu.x)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.x>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void iny (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"iny ");
#endif 
    if (cpu.y!=0xFF) cpu.y++; 
    else cpu.y=0;

    if (!cpu.y)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.y>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void jmp (unsigned char mode) 
{
    unsigned char lowbyte, highbyte;
#ifdef DEBUG
    fprintf(stderr,"jmp ");
#endif 
    if (mode==ABSOLUTE) 
    {
        lowbyte=fetchmemory();
        highbyte=fetchmemory();
        cpu.pc= (unsigned short) (highbyte<<8) | lowbyte;
    }
    else
    {
        cpu.pc = get_address(mode);
    }
}

__attribute((always_inline)) inline void jsr (unsigned char mode) 
{
    unsigned char operand_l, operand_h;
    operand_l = (char) (cpu.pc+1);
    operand_h = (char) ((cpu.pc+1)>>8);
    writememory(0x100+cpu.sp, operand_h);
    cpu.sp--;
    writememory(0x100+cpu.sp, operand_l);
    cpu.sp--;
	operand_l = fetchmemory();
	operand_h = fetchmemory();
	cpu.pc = (unsigned short) (operand_h << 8) | operand_l;
    used=1;
#ifdef DEBUG
    fprintf(stderr,"jsr %04X ", cpu.pc);
#endif 
}

__attribute((always_inline)) inline void lda (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"lda ");
#endif 
    if (mode==IMMEDIATE) cpu.a=fetchmemory(); 
    else cpu.a=readmemory(get_address(mode));

    if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void ldx (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"ldx ");
#endif 
    if (mode==IMMEDIATE) cpu.x=fetchmemory(); 
    else cpu.x=readmemory(get_address(mode));

    if (!cpu.x)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.x>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void ldy (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"ldy ");
#endif 
    if (mode==IMMEDIATE) cpu.y=fetchmemory(); 
    else cpu.y=readmemory(get_address(mode));

    if (!cpu.y)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.y>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void lsr (unsigned char mode) 
{
    unsigned short val;
    unsigned short aux;
#ifdef DEBUG
    fprintf(stderr,"lsr ");
#endif 
    if (mode==ACCUMULATOR)
    {
        cpu.status = (cpu.status & ~(1UL << 0)) | (cpu.a & 1UL << 0); // set bit carry on status processor to accumulator bit zero
        cpu.a = cpu.a >> 1;       
        if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
        if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
    }
    else
    {
        aux = get_address(mode);
        val = readmemory(aux);
        cpu.status = (cpu.status & ~(1UL << 0)) | (val & 1UL << 0); // set bit carry on status processor to memory bit zero
        val = val >> 1;
        writememory ( aux, val );
        if (!val)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1); // set bit zero on status processor to true
        if (val>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7); // set bit negative on status processor to true
    }
}

__attribute((always_inline)) inline void nop (unsigned char mode)
{
    // do nothing
#ifdef DEBUG
    fprintf(stderr,"nop ");
#endif 
    return;
}

__attribute((always_inline)) inline void ora (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"ora ");
#endif 
    if (mode==IMMEDIATE) cpu.a = cpu.a | fetchmemory();
    else cpu.a = cpu.a | readmemory(get_address(mode));
     
    if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void pha (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"pha ");
#endif 
    writememory(0x100+cpu.sp, cpu.a);
    if (cpu.sp>0) cpu.sp--;
    else cpu.sp=0xFF;
}

__attribute((always_inline)) inline void php (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"php ");
#endif 
    writememory(0x100+cpu.sp, cpu.status | 0x30);  // set bits break and reserved to true on the stack copy of the status register
    if (cpu.sp>0) cpu.sp--;
    else cpu.sp=0xFF;
}

__attribute((always_inline)) inline void pla (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"pla ");
#endif 
    if (cpu.sp<0xFF) cpu.sp++;
    else cpu.sp=0;
    cpu.a = readmemory(0x100+cpu.sp);

    if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor
    if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void plp (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"plp ");
#endif 
    if (cpu.sp<0xFF) cpu.sp++;
    else cpu.sp=0;
    cpu.status = readmemory(0x100+cpu.sp) & 0xEF; //unset break flag
}

__attribute((always_inline)) inline void rol (unsigned char mode) 
{
    unsigned char tmp;
    unsigned short aux;
    unsigned char val;
#ifdef DEBUG
    fprintf(stderr,"rol ");
#endif 
    if (mode==ACCUMULATOR)
    {
        tmp = cpu.status;
        cpu.status = (cpu.status & ~(1UL << 0)) | ((cpu.a & (1UL << 7)) >> 7); // set bit carry on status processor to bit 7 of accumulator
        cpu.a = cpu.a << 1;       
        cpu.a = (cpu.a & ~(1UL << 0)) | (tmp & (1UL << 0)); // set bit zero on accumulator to previous carry
        if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
        if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
    }
    else
    {
        aux = get_address(mode);
        val = readmemory(aux);
        tmp = cpu.status;
        cpu.status = (cpu.status & ~(1UL << 0)) | ((val & (1UL << 7)) >> 7); // set bit carry on status processor to bit 7 of memory
        val = val << 1;       
        val = (val & ~(1UL << 0)) | (tmp & (1UL << 0)); // set bit zero on memory to previous carry
        writememory(aux, val); 
        if (!val)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1); // set bit zero on status processor to true
        if (val>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7); // set bit negative on status processor to true
    }
}

__attribute((always_inline)) inline void ror (unsigned char mode) 
{
    unsigned char tmp;
    unsigned short aux;
    unsigned char val;
#ifdef DEBUG
    fprintf(stderr,"ror ");
#endif 
    if (mode==ACCUMULATOR)
    {
        tmp = cpu.status;
        cpu.status = (cpu.status & ~(1UL << 0)) | (cpu.a & (1UL << 0)); // set bit carry on status processor to bit 0 of accumulator
        cpu.a = cpu.a >> 1;       
        cpu.a = (cpu.a & ~(1UL << 7)) | ((tmp & (1UL << 0)) << 7); // set bit 7 on accumulator to previous carry
        if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
        if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
    }
    else
    {
        aux = get_address(mode);
        val = readmemory(aux);
        tmp = cpu.status;
        cpu.status = (cpu.status & ~(1UL << 0)) | (val & (1UL << 0)); // set bit carry on status processor to bit 0 of memory
        val = val >> 1;       
        val = (val & ~(1UL << 7)) | ((tmp & (1UL << 0)) << 7); // set bit 7 on memory to previous carry
        writememory(aux, val);
        if (!val)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1); // set bit zero on status processor to true
        if (val>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7); // set bit negative on status processor to true
    }
}

__attribute((always_inline)) inline void rti (unsigned char mode) 
{
    unsigned char operand_l, operand_h;
#ifdef DEBUG
    fprintf(stderr,"rti ");
#endif
    cpu.sp++;
    cpu.status = readmemory(0x100+cpu.sp) & 0xEF; // clear bits 4 and 5 when restablishing the status register
    cpu.sp++;
    operand_l = readmemory(0x100+cpu.sp);
    cpu.sp++;
    operand_h = readmemory(0x100+cpu.sp);
    cpu.pc = (unsigned short) ((operand_h<<8) | (operand_l));
}

__attribute((always_inline)) inline void rts (unsigned char mode) 
{
    unsigned char operand_l, operand_h;
#ifdef DEBUG
    fprintf(stderr,"rts ");
#endif 
    cpu.sp++;
    operand_l = readmemory(0x100+cpu.sp);
    cpu.sp++;
    operand_h = readmemory(0x100+cpu.sp);
    cpu.pc = (unsigned short) ((operand_h<<8) | (operand_l)) + 1;
}

__attribute((always_inline)) inline void sbc (unsigned char mode) 
{
    unsigned short sum; 
    unsigned char operand;
    unsigned char al, ah, ol, oh, cl;
    unsigned char difl, difh;
#ifdef DEBUG
    fprintf(stderr,"sbc ");
#endif 
    if (mode==IMMEDIATE) operand = fetchmemory();
    else operand = readmemory(get_address(mode));

    if ((cpu.status & 1UL<<3)>>3) { 
        al = cpu.a & 0x0F;
        ol = operand & 0x0F;
        ah = cpu.a & 0xF0;
        oh = operand & 0xF0;
        cl = 0;
        if (cpu.status & 1UL<<0) {
            if (ol>al) { 
                difl = al + 0x0A - ol;
                cl = 0x10;
            }
            else {
                difl = al - ol;
            }
            if (oh+cl>ah) {
                difh = ah + 0xA0 - oh - cl;
                cpu.status &= ~(1UL << 0);  // clear bit carry on status processor
            }        
            else 
            {
                difh = ah - oh - cl;
                cpu.status |= 1UL << 0;     // set bit carry on status processor
            }   
            sum = difh + difl;
        }
        else {
            if ((ol+1)>al) { 
                difl = al + 0x0A - ol - 1;
                cl = 0x10;
            }
            else {
                difl = al - ol - 1;
            }
            if (oh+cl>ah) {
                difh = ah + 0xA0 - oh - cl;
                cpu.status &= ~(1UL << 0);  // clear bit carry on status processor
            }        
            else 
            {
                difh = ah - oh -cl;
                cpu.status |= 1UL << 0;     // set bit carry on status processor
            }   
            sum = difh + difl;
        }
        cpu.a = (char) sum; 
    }
    else {
        operand ^= 0xFFU;
        if (cpu.status & 1UL<<0) sum = cpu.a + operand + 1; 
        else                     sum = cpu.a + operand;
        if (sum>0xFF)    cpu.status |= 1UL << 0; else cpu.status &= ~(1UL << 0);  // set bit carry on status processor
        if ((!((cpu.a ^ operand) & 0x80) && ((cpu.a ^ sum) & 0x80))!=0) cpu.status |= 1UL << 6; else cpu.status &= ~(1UL << 6); // set bit overflow   
        cpu.a = (char) sum; 
        if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
        if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
    }

}

__attribute((always_inline)) inline void sec (unsigned char mode)
{
#ifdef DEBUG
    fprintf(stderr,"sec ");
#endif 
    cpu.status |= 1UL << 0;     // set bit carry on status processor to true
}

__attribute((always_inline)) inline void sed (unsigned char mode)
{
#ifdef DEBUG
    fprintf(stderr,"sed ");
#endif 
    cpu.status |= 1UL << 3;     // set bit decimal on status processor to true
}

__attribute((always_inline)) inline void sei (unsigned char mode)
{
#ifdef DEBUG
    fprintf(stderr,"sei ");
#endif 
    cpu.status |= 1UL << 2;     // set bit interrupt on status processor to true (interrupt disabled)
}

__attribute((always_inline)) inline void sta (unsigned char mode) 
{
    int addr;
#ifdef DEBUG
    fprintf(stderr,"sta ");
#endif 
    addr = get_address(mode);
	writememory(addr, cpu.a);
}

__attribute((always_inline)) inline void stx (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"stx ");
#endif 
	writememory(get_address(mode), cpu.x);
}

__attribute((always_inline)) inline void sty (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"sty ");
#endif 
	writememory(get_address(mode), cpu.y);
}

__attribute((always_inline)) inline void tax (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"tax ");
#endif 
    cpu.x = cpu.a;
    if (!cpu.x)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.x>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void tay (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"tay ");
#endif 
    cpu.y = cpu.a;
    if (!cpu.y)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.y>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void tsx (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"tsx ");
#endif 
    cpu.x = cpu.sp;
    if (!cpu.x)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.x>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void txa (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"txa ");
#endif 
    cpu.a = cpu.x;
    if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

__attribute((always_inline)) inline void txs (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"txs ");
#endif 
    cpu.sp = cpu.x;
}

__attribute((always_inline)) inline void tya (unsigned char mode) 
{
#ifdef DEBUG
    fprintf(stderr,"tya ");
#endif 
    cpu.a = cpu.y;
    if (!cpu.a)      cpu.status |= 1UL << 1; else cpu.status &= ~(1UL << 1);  // set bit zero on status processor 
    if (cpu.a>=0x80) cpu.status |= 1UL << 7; else cpu.status &= ~(1UL << 7);  // set bit negative on status processor
}

// 
// Switch case to execute CPU command based on opcode
//
int processcommand()
{ 
    const unsigned char length[256]= { 7, 6, 2, 2, 2, 3, 5, 2, 3, 2, 2, 2, 2, 4, 6, 2,  // 00
                                       2, 5, 2, 2, 2, 4, 6, 2, 2, 4, 2, 2, 2, 4, 7, 2,  // 10
                                       6, 6, 2, 2, 3, 3, 5, 2, 4, 2, 2, 2, 4, 4, 6, 2,  // 20
                                       2, 5, 2, 2, 2, 4, 6, 2, 2, 4, 2, 2, 2, 4, 7, 2,  // 30
                                       6, 6, 2, 2, 2, 3, 5, 2, 3, 2, 2, 2, 3, 4, 6, 2,  // 40
                                       2, 5, 2, 2, 2, 4, 6, 2, 2, 4, 2, 2, 2, 4, 7, 2,  // 50
                                       6, 6, 2, 2, 2, 3, 5, 2, 4, 2, 2, 2, 5, 4, 6, 2,  // 60
                                       2, 5, 2, 2, 2, 4, 6, 2, 2, 4, 2, 2, 2, 4, 7, 2,  // 70
                                       2, 6, 2, 2, 3, 3, 3, 2, 2, 2, 2, 2, 4, 4, 4, 2,  // 80
                                       2, 6, 2, 2, 4, 4, 4, 2, 2, 5, 2, 2, 2, 5, 2, 2,  // 90
                                       2, 6, 2, 2, 3, 3, 3, 2, 2, 2, 2, 2, 4, 4, 4, 2,  // A0
                                       2, 5, 2, 2, 4, 4, 4, 2, 2, 4, 2, 2, 4, 4, 4, 2,  // B0
                                       2, 6, 2, 2, 3, 3, 5, 2, 2, 2, 2, 2, 4, 4, 6, 2,  // C0
                                       2, 5, 2, 2, 2, 4, 6, 2, 2, 4, 2, 2, 2, 4, 7, 2,  // D0
                                       2, 6, 2, 2, 3, 3, 5, 2, 2, 2, 2, 2, 4, 4, 6, 2,  // E0
                                       2, 5, 2, 2, 2, 4, 6, 2, 2, 4, 2, 2, 2, 4, 7, 2 };// F0
    unsigned char command;

    bordercross = 0;
    command = fetchmemory();
    cpu.cycles += length[command];

#ifdef DEBUG
    fprintf (stderr, "%2X ", command);
#endif 
    
    switch (command)
    {
        case 0x69: adc(IMMEDIATE); break;
        case 0x65: adc(ZERO_PAGE); break;
        case 0x75: adc(ZERO_PAGE_X); break;
        case 0x6D: adc(ABSOLUTE); break;
        case 0x7D: adc(ABSOLUTE_X); cpu.cycles += bordercross; break;
        case 0x79: adc(ABSOLUTE_Y); cpu.cycles += bordercross; break;
        case 0x61: adc(INDIRECT_X); break;
        case 0x71: adc(INDIRECT_Y); cpu.cycles += bordercross; break;

        case 0x29: fand(IMMEDIATE); break;
        case 0x25: fand(ZERO_PAGE); break;
        case 0x35: fand(ZERO_PAGE_X); break;
        case 0x2D: fand(ABSOLUTE); break;
        case 0x3D: fand(ABSOLUTE_X); cpu.cycles += bordercross; break;
        case 0x39: fand(ABSOLUTE_Y); cpu.cycles += bordercross; break;
        case 0x21: fand(INDIRECT_X); break;
        case 0x31: fand(INDIRECT_Y); cpu.cycles += bordercross; break;
        
        case 0x0A: asl(ACCUMULATOR); break;
        case 0x06: asl(ZERO_PAGE); break;
        case 0x16: asl(ZERO_PAGE_X); break;
        case 0x0E: asl(ABSOLUTE); break;
        case 0x1E: asl(ABSOLUTE_X); break;

        case 0x90: bcc(RELATIVE); break;
        case 0xB0: bcs(RELATIVE); break;
        case 0xF0: beq(RELATIVE); break;
        case 0x30: bmi(RELATIVE); break;
        case 0xD0: bne(RELATIVE); break;
        case 0x10: bpl(RELATIVE); break;
        case 0x50: bvc(RELATIVE); break;
        case 0x70: bvs(RELATIVE); break;

        case 0x24: bit(ZERO_PAGE); break;
        case 0x2C: bit(ABSOLUTE); break;

        case 0x00: fbrk(IMPLIED); break;

        case 0x18: clc(IMPLIED); break;
        case 0xD8: cld(IMPLIED); break;
        case 0x58: cli(IMPLIED); break;
        case 0xB8: clv(IMPLIED); break;

        case 0xC9: cmp(IMMEDIATE); break;
        case 0xC5: cmp(ZERO_PAGE); break;
        case 0xD5: cmp(ZERO_PAGE_X); break;
        case 0xCD: cmp(ABSOLUTE); break;
        case 0xDD: cmp(ABSOLUTE_X); cpu.cycles += bordercross; break;
        case 0xD9: cmp(ABSOLUTE_Y); cpu.cycles += bordercross; break;
        case 0xC1: cmp(INDIRECT_X); break;
        case 0xD1: cmp(INDIRECT_Y); cpu.cycles += bordercross; break;

        case 0xE0: cpx(IMMEDIATE); break;
        case 0xE4: cpx(ZERO_PAGE); break;
        case 0xEC: cpx(ABSOLUTE); break;

        case 0xC0: cpy(IMMEDIATE); break;
        case 0xC4: cpy(ZERO_PAGE); break;
        case 0xCC: cpy(ABSOLUTE); break;

        case 0xC6: dec(ZERO_PAGE); break;
        case 0xD6: dec(ZERO_PAGE_X); break;
        case 0xCE: dec(ABSOLUTE); break;
        case 0xDE: dec(ABSOLUTE_X); break;

        case 0xCA: dex(IMPLIED); break;
        case 0x88: dey(IMPLIED); break;

        case 0x49: eor(IMMEDIATE); break;
        case 0x45: eor(ZERO_PAGE); break;
        case 0x55: eor(ZERO_PAGE_X); break;
        case 0x4D: eor(ABSOLUTE); break;
        case 0x5D: eor(ABSOLUTE_X); cpu.cycles += bordercross; break;
        case 0x59: eor(ABSOLUTE_Y); cpu.cycles += bordercross; break;
        case 0x41: eor(INDIRECT_X); break;
        case 0x51: eor(INDIRECT_Y); cpu.cycles += bordercross; break;

        case 0xE6: inc(ZERO_PAGE); break;
        case 0xF6: inc(ZERO_PAGE_X); break;
        case 0xEE: inc(ABSOLUTE); break;
        case 0xFE: inc(ABSOLUTE_X); break;

        case 0xE8: inx(IMPLIED); break;
        case 0xC8: iny(IMPLIED); break;

        case 0x4C: jmp(ABSOLUTE); break;
        case 0x6C: jmp(INDIRECT); break;

        case 0x20: jsr(ABSOLUTE); break;

        case 0xA1: lda(INDIRECT_X); break;
        case 0xA5: lda(ZERO_PAGE); break;
        case 0xA9: lda(IMMEDIATE); break;
        case 0xAD: lda(ABSOLUTE); break;
        case 0xB1: lda(INDIRECT_Y); cpu.cycles += bordercross; break;
        case 0xB5: lda(ZERO_PAGE_X); break;
        case 0xBD: lda(ABSOLUTE_X); cpu.cycles += bordercross; break;
        case 0xB9: lda(ABSOLUTE_Y); cpu.cycles += bordercross; break;

        case 0xA2: ldx(IMMEDIATE); break;
        case 0xA6: ldx(ZERO_PAGE); break;
        case 0xB6: ldx(ZERO_PAGE_Y); break;
        case 0xAE: ldx(ABSOLUTE); break;
        case 0xBE: ldx(ABSOLUTE_Y); cpu.cycles += bordercross; break;

        case 0xA0: ldy(IMMEDIATE); break;
        case 0xA4: ldy(ZERO_PAGE); break;
        case 0xB4: ldy(ZERO_PAGE_X); break;
        case 0xAC: ldy(ABSOLUTE); break;
        case 0xBC: ldy(ABSOLUTE_X); cpu.cycles += bordercross; break;

        case 0x4A: lsr(ACCUMULATOR); break;
        case 0x46: lsr(ZERO_PAGE); break;
        case 0x56: lsr(ZERO_PAGE_X); break;
        case 0x4E: lsr(ABSOLUTE); break;
        case 0x5E: lsr(ABSOLUTE_X); break;

        case 0xEA: nop(IMPLIED); break;

        case 0x09: ora(IMMEDIATE); break;
        case 0x05: ora(ZERO_PAGE); break;
        case 0x15: ora(ZERO_PAGE_X); break;
        case 0x0D: ora(ABSOLUTE); break;
        case 0x1D: ora(ABSOLUTE_X); cpu.cycles += bordercross; break;
        case 0x19: ora(ABSOLUTE_Y); cpu.cycles += bordercross; break;
        case 0x01: ora(INDIRECT_X); break;
        case 0x11: ora(INDIRECT_Y); cpu.cycles += bordercross; break;
        
        case 0x48: pha(IMPLIED); break;
        case 0x08: php(IMPLIED); break;
        case 0x68: pla(IMPLIED); break;
        case 0x28: plp(IMPLIED); break;

        case 0x2A: rol(ACCUMULATOR); break;
        case 0x26: rol(ZERO_PAGE); break;
        case 0x36: rol(ZERO_PAGE_X); break;
        case 0x2E: rol(ABSOLUTE); break;
        case 0x3E: rol(ABSOLUTE_X); break;

        case 0x6A: ror(ACCUMULATOR); break;
        case 0x66: ror(ZERO_PAGE); break;
        case 0x76: ror(ZERO_PAGE_X); break;
        case 0x6E: ror(ABSOLUTE); break;
        case 0x7E: ror(ABSOLUTE_X); break;

        case 0x40: rti(IMPLIED); break;

        case 0x60: rts(IMPLIED); break;

        case 0xE9: sbc(IMMEDIATE); break;
        case 0xE5: sbc(ZERO_PAGE); break;
        case 0xF5: sbc(ZERO_PAGE_X); break;
        case 0xED: sbc(ABSOLUTE); break;
        case 0xFD: sbc(ABSOLUTE_X); cpu.cycles += bordercross; break;
        case 0xF9: sbc(ABSOLUTE_Y); cpu.cycles += bordercross; break;
        case 0xE1: sbc(INDIRECT_X); break;
        case 0xF1: sbc(INDIRECT_Y); cpu.cycles += bordercross; break;

        case 0x38: sec(IMPLIED); break;
        case 0xF8: sed(IMPLIED); break;
        case 0x78: sei(IMPLIED); break;

        case 0x81: sta(INDIRECT_X); break; 
        case 0x85: sta(ZERO_PAGE); break;
        case 0x8D: sta(ABSOLUTE); break;
        case 0x95: sta(ZERO_PAGE_X); break;
        case 0x9D: sta(ABSOLUTE_X); break;
        case 0x99: sta(ABSOLUTE_Y); break;
        case 0x91: sta(INDIRECT_Y); break;

        case 0x86: stx(ZERO_PAGE); break;
        case 0x96: stx(ZERO_PAGE_Y); break;
        case 0x8E: stx(ABSOLUTE); break;

        case 0x84: sty(ZERO_PAGE); break;
        case 0x94: sty(ZERO_PAGE_X); break;
        case 0x8C: sty(ABSOLUTE); break;

        case 0xAA: tax(IMPLIED); break;
        case 0xA8: tay(IMPLIED); break;
        case 0xBA: tsx(IMPLIED); break;
        case 0x8A: txa(IMPLIED); break;
        case 0x9A: txs(IMPLIED); break;
        case 0x98: tya(IMPLIED); break;

        default: nop(IMPLIED); break;

    }
    return 0;
}

void interrupt ()
{
    unsigned char operand_l, operand_h;
#ifdef DEBUG
    fprintf(stderr,"External Interrupt ");
#endif 
    if (!(cpu.status&0x04)) {
        operand_l = (char) (cpu.pc+1);
        operand_h = (char) ((cpu.pc+1)>>8);
        writememory(0x100+cpu.sp, operand_h);
        cpu.sp--;
        writememory(0x100+cpu.sp, operand_l);
        cpu.sp--;
        writememory(0x100+cpu.sp, cpu.status | 0x20);  // set bits break and reserved to true on the stack copy of the status register
        cpu.sp--;
        cpu.status |= 0x04;
        operand_l = readmemory(0xFFFE);
        operand_h = readmemory(0xFFFF);
        cpu.pc = (unsigned short) ((operand_h<<8) | (operand_l));
        cpu.status |= 0x04;
    }
}

void nmi ()
{
    unsigned char operand_l, operand_h;
#ifdef DEBUG
    fprintf(stderr,"External Non-Maskable Interrupt ");
#endif 
    operand_l = (char) (cpu.pc+1);
    operand_h = (char) ((cpu.pc+1)>>8);
    writememory(0x100+cpu.sp, operand_h);
    cpu.sp--;
    writememory(0x100+cpu.sp, operand_l);
    cpu.sp--;
    writememory(0x100+cpu.sp, cpu.status | 0x20);  // set bits break and reserved to true on the stack copy of the status register
    cpu.sp--;
    cpu.status |= 0x04;
    operand_l = readmemory(0xFFFA);
    operand_h = readmemory(0xFFFB);
    cpu.pc = (unsigned short) ((operand_h<<8) | (operand_l));
    cpu.status |= 0x04;
}

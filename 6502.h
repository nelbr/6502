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
//    along with lib6502.  If not, see <https://www.gnu.org/licenses/>.
//
//    nelbr - Summer 2020
//

#ifndef MOS_H
#define MOS_H

// #define DEBUG 1

#define STATUS_TO_BINARY_PATTERN "     Ne %c Ov %c NA %c Br %c De %c In %c Ze %c Ca %c\n"
#define STATUS_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

struct microprocessor {
	unsigned char a;
	unsigned char x;
	unsigned char y;
    unsigned char sp;
    unsigned short pc;
	unsigned char status;
    unsigned long cycles;
} cpu;

unsigned int used;

int processcommand();
void interrupt();
void nmi();
extern unsigned char readmemory(unsigned short);
extern void writememory(unsigned short, unsigned char);

#endif

#ifndef MOS_H
#define MOS_H

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

typedef struct microprocessor {
	unsigned char a;
	unsigned char x;
	unsigned char y;
    unsigned char sp;
    unsigned short pc;
	unsigned char status;
    unsigned long cycles;
} microprocessor;

unsigned char fetchmemory();
int processcommand();
extern unsigned char readmemory(unsigned short);
extern void writememory(unsigned short, unsigned char);

#endif

// #ifndef 6502_H_   /* Include guard */
// #define 6502_H_

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
    unsigned int cycles;
} microprocessor;

unsigned char fetchmemory();
unsigned short get_address( unsigned short int mode);
int processcommand();
void adc (short);
void fand (short);
void asl (short);
void bcc (short);
void bcs (short);
void beq (short);
void bit (short);
void bmi (short);
void bne (short);
void bpl (short);
void fbrk (short);
void bvc (short);
void bvs (short);
void clc (short);
void cld (short);
void cli (short);
void clv (short);
void cmp (short);
void cpx (short);
void cpy (short);
void dec (short);
void dex (short);
void dey (short);
void eor (short);
void inc (short);
void inx (short);
void iny (short);
void jmp (short);
void jsr (short);
void lda (short);
void ldx (short);
void ldy (short);
void lsr (short);
void nop (short);
void ora (short);
void pha (short);
void php (short);
void pla (short);
void plp (short);
void rol (short);
void ror (short);
void rti (short);
void rts (short);
void sbc (short);
void sec (short);
void sed (short);
void sei (short);
void sta (short);
void stx (short);
void sty (short);
void tax (short);
void tay (short);
void tsx (short);
void txa (short);
void txs (short);
void tya (short);
extern unsigned char readmemory(unsigned short);
extern void writememory(unsigned short, unsigned char);

// #endif // 6502_H_

// ANSI Codes

#define ANSI_ESCAPE          0x1b
#define ANSI_SEPARATOR       0x3b // ;
#define ANSI_BRACKET         0x5b // [
#define ANSI_CURSOR_UP       0x41 // A
#define ANSI_CURSOR_DOWN     0x42 // B
#define ANSI_CURSOR_FORWARD  0x43 // C
#define ANSI_CURSOR_BACKWARD 0x44 // D
#define ANSI_CURSOR_HOME     0x48 // H
#define ANSI_CLEAR_SCREEN    0x4A // J
#define ANSI_CLEAR_LINE      0x4B // K
#define ANSI_GRAPHICS_MODE   0x6D // m
#define ANSI_DECSTBM         0x72 // r
#define ANSI_PRIVATE         0x3f // ?
#define ANSI_DEC_h           0x68 // h
#define ANSI_DEC_l           0x68 // h
#define ANSI_HPA	         0x62 // b
#define ANSI_VPA	         0x64 // d

// PETSCII/ASCII Character codes
#define DELETE			0x14
#define CLRSCR			0x93
#define HOME			0x13
#define BELL			0x07
#define	CR				0x0d
#define SHIFT_CR		0x8d
#define LF				0x0a
#define PIPE            0xdd

#define ANSI_VALUE_BUFFER_SIZE 10

void send3chars(unsigned char c1, unsigned char c2, unsigned char c3);
void putchar_ansi(char ac);
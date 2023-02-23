#include "ansi.h"

char buff[ANSI_VALUE_BUFFER_SIZE];

void send3chars(unsigned char c1, unsigned char c2, unsigned char c3) 
{
	buff[0] = c1;
	buff[1] = c2;
	buff[2] = c3;
	buff[3] = 0;
	uii_socketwrite(socketnr, buff);
}

void putchar_ansi(char ac)
{
	switch (ac)
	{
		case LF:     // Ignore linefeeds
			break;

		case BELL:   // Ding!
	//		term_bell();
			break;

		case ANSI_ESCAPE:
			//str = parse_ansi_escape(str++);
			break;

		default:
			putchar(ascToPet[ac]);
			break;
		}
	}
}
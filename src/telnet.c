// Telnet Stuff
#include "telnet.h"
#include "weeip.h"

int telnet_state = TELNET_STATE_INIT;

void send_char(char c)
{
	socket_send(c, 1);
}

void SendTelnetDoWill(unsigned char verb, unsigned char opt)
{
	send_char(NVT_IAC);                               // send character 255 (start negotiation)
	send_char(verb == NVT_DO ? NVT_DO : NVT_WILL);    // send character 253  (do) if negotiation verb character was 253 (do) else send character 251 (will)
	send_char(opt);
}

void SendTelnetDontWont(unsigned char verb, unsigned char opt)
{
	send_char(NVT_IAC);                               // send character 255   (start negotiation)
	send_char(verb == NVT_DO ? NVT_WONT : NVT_DONT);  // send character 252   (wont) if negotiation verb character was 253 (do) else send character 254 (dont)
	send_char(opt);
}

void SendTelnetParameters()
{
	send_char(NVT_IAC);                               // send character 255 (start negotiation) 
	send_char(NVT_DONT);                              // send character 254 (dont)
	send_char(NVT_OPT_LINEMODE);                      // linemode

	send_char(NVT_IAC);                               // send character 255 (start negotiation)
	send_char(NVT_DO);                                // send character 253 (do)
	send_char(NVT_OPT_ECHO);                          // echo
}

// Returns 1 (true) if a second IAC is returned
unsigned char handle_telnet_iac() {

	int datacount;
	unsigned char verb;           // telnet parameters
	unsigned char opt;

	datacount = uii_socketread(socketnr, 1);   // TODO check for closed connection
	verb = uii_data[2];                                // receive negotiation verb character

	if (verb == NVT_IAC)
	{
		return 1;                                  // Received two NVT_IACs in a row so treat as single 255 data in calling function
	}

	datacount = uii_socketread(socketnr, 1);   // TODO check for closed connection
	opt = uii_data[2];                               // receive negotiation option character

	switch (verb)                                  // evaluate negotiation verb character
	{
		case NVT_WILL:                                      // if negotiation verb character is 251 (will)or
		case NVT_DO:                                        // if negotiation verb character is 253 (do) or
			switch (opt) 
			{
				case NVT_OPT_SUPPRESS_GO_AHEAD:                 // if negotiation option character is 3 (suppress - go - ahead)
					SendTelnetDoWill(verb, opt);
					break;

				case NVT_OPT_TRANSMIT_BINARY:                   // if negotiation option character is 0 (binary data)
					SendTelnetDoWill(verb, opt);					
					break;

				default:                                        // if negotiation option character is none of the above(all others), just say no
					SendTelnetDontWont(verb, opt);
					break;                                      //  break the routine
			}
			break;

		case NVT_WONT:                                      // if negotiation verb character is 252 (wont)or
		case NVT_DONT:                                      // if negotiation verb character is 254 (dont)
			switch (opt) 
			{
				case NVT_OPT_TRANSMIT_BINARY:                   // if negotiation option character is 0 (binary data)
					SendTelnetDontWont(verb, opt);
					break;

				default:                                        // if negotiation option character is none of the above(all others)
					SendTelnetDontWont(verb, opt);
					break;                                      //  break the routine
			}
			break;

		case NVT_IAC:                                       // Ignore second IAC/255 if we are not in BINARY mode
			break;

		default:
			//printf(">> Unknown IAC Verb  %d\n", verb);
			break;
	}
	return 0;
}


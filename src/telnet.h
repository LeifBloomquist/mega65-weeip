// Telnet Stuff

#define NVT_SE 240
#define NVT_NOP 241
#define NVT_DATAMARK 242
#define NVT_BRK 243
#define NVT_IP 244
#define NVT_AO 245
#define NVT_AYT 246
#define NVT_EC 247
#define NVT_GA 249
#define NVT_SB 250
#define NVT_WILL 251
#define NVT_WONT 252
#define NVT_DO 253
#define NVT_DONT 254
#define NVT_IAC 255

#define NVT_OPT_TRANSMIT_BINARY 0
#define NVT_OPT_ECHO 1
#define NVT_OPT_SUPPRESS_GO_AHEAD 3
#define NVT_OPT_STATUS 5
#define NVT_OPT_RCTE 7
#define NVT_OPT_TIMING_MARK 6
#define NVT_OPT_NAOCRD 10
#define NVT_OPT_TERMINAL_TYPE 24
#define NVT_OPT_NAWS 31
#define NVT_OPT_TERMINAL_SPEED 32
#define NVT_OPT_LINEMODE 34
#define NVT_OPT_X_DISPLAY_LOCATION 35
#define NVT_OPT_ENVIRON 36
#define NVT_OPT_NEW_ENVIRON 39

#define TELNET_STATE_INIT  0
#define TELNET_STATE_VERB  1
#define TELNET_STATE_OPT   2

void SendTelnetDoWill(unsigned char verb, unsigned char opt);
void SendTelnetDontWont(unsigned char verb, unsigned char opt);
void SendTelnetParameters();
unsigned char handle_telnet_iac();
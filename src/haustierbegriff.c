#include <stdio.h>
#include <string.h>
#include <conio.h>

// WeeIP
#include "task.h"
#include "weeip.h"
#include "eth.h"
#include "arp.h"
#include "dns.h"
#include "dhcp.h"

// Terminal support
#include "telnet.h"
#include "ansi.h"

// MEGA65-clib
#include "memory.h"
#include "random.h"

#define SCREEN_BASE 0xC000

// Keys
#define KEY_HELP  31
#define KEY_F7    247
#define KEY_F9    249

// Character Modes
#define MODE_PETSCII 0
#define MODE_ASCII   1
#define MODE_ANSI    2

unsigned char character_mode = MODE_PETSCII;

// Screen Modes
enum screenmodes { SCREEN_80x25, SCREEN_80x50, SCREEN_40x25, SCREEN_LAST } screenmode;
char screen_mode_strings[3][6] = { "80x25", "80x50", "40x25" };

// Global Variables
unsigned char last_frame_number=0;
unsigned long byte_log=0;
char tempstring[100];
unsigned char nbbs = 0;
SOCKET* s;
byte_t buf[1500];
char disconnected = FALSE;

// Telnet Flags
unsigned char first_char = 0;
int telnet_state = TELNET_STATE_INIT;
unsigned char telnet_verb;
unsigned char telnet_opt;

struct bbs {
  char *name;
  char *host_name;
  unsigned int port_number;
};

const struct bbs bbs_list[27]=
  {
   {"Boar's Head","byob.hopto.org",64128},
   {"RapidFire","rapidfire.hopto.org",64128},
   {"Antidote by Triad","antidote.hopto.org",64128},
   {"Wizards's Realm", "wizardsrealm.c64bbs.org", 23},
   {"The Hidden", "the-hidden.hopto.org", 64128},
   {"Eaglewing BBS", "eagelbird.ddns.net", 6400},
   {"Scorps Portal", "scorp.us.to", 23},
   {"My C=ult BBS", "maraud.dynalias.com", 6400},
   {"Local Ubuntu", "192.168.7.51", 23},
   {"Netcat", "192.168.7.51", 5000},
   {"The Hidden IP", "146.0.33.231", 64128},
   /*
   {"Commodore Image", "cib.dyndns.org", 6400},
   {"64 Vintage Remix", "64vintageremixbbs.dyndns.org", 6400},
   {"Centronian BBS", "centronian.servebeer.com", 6400},
   {"Anrchy Undergrnd", "aubbs.dyndns.org", 2300},
   {"The Oasis BBS", "oasisbbs.hopto.org", 6400},
   {"The Disk Box", "bbs.thediskbox.com", 6400},
   {"Cottonwood", "cottonwoodbbs.dyndns.org", 6502},
   {"Wrong Number ][", "cib.dyndns.org", 6404},
   {"RabidFire", "rapidfire.hopto.org", 64128},
   {"Mad World", "madworld.bounceme.net", 6400},
   {"Citadel 64", "bbs.thejlab.com", 6400},
   {"Hotwire BBS", "hotwirebbs.zapto.org", 6502},
   {"Endless Chaos", "endlesschaos.dyndns.org", 6400},
   {"Borderline", "borderlinebbs.dyndns.org", 6400},
   {"RAVELOUTION","raveolution.hopto.org",64128},
   {"The Edge BBS","theedgebbs.dyndns.org",1541},
   {"PGS Test","F96NG92-L.fritz.box",64128},
   */
   {NULL,NULL,0}
  };

void dump_bytes(char* msg, uint8_t* d, int count);

/* Handle a single incoming character. */
void handle_rx(char c)
{
    sprintf(tempstring, "{lgn}[$%02x]", c);
    pcprintf(tempstring);
    
    // Special handling for first char.   If first character back from remote side is NVT_IAC, we have a telnet connection.  Force ANSI mode
    if (first_char && (c == NVT_IAC))
    {        
        SendTelnetParameters();
        telnet_state = TELNET_STATE_VERB;
        //bordercolor(COLOUR_RED);
        character_mode = MODE_ANSI;
        first_char = 0;
        //cursor_off();
        pcprintf("{lgrn}(Telnet){wht}\n");
        //cursor_on();
        return;
    }

    if (c == NVT_IAC) // Subsequent IACs
    {
        telnet_state = TELNET_STATE_VERB;
        return;
    }

    switch (telnet_state)
    {
        case TELNET_STATE_VERB:
            pcprintf("{yel}(Verb){wht}\n");
            telnet_verb = c;
            telnet_state = TELNET_STATE_OPT;
            return;

        case TELNET_STATE_OPT:
            pcprintf("{blu}(Opt){wht}\n");
            telnet_opt = c;
            //handle_telnet_iac(telnet_verb, telnet_opt);
            telnet_state = TELNET_STATE_INIT;
            return;
    }
    
    //bordercolor(COLOUR_DARKGREY);

    //  Finally regular data - just display
    switch (character_mode)
    {
        case MODE_PETSCII:
            pcputc(c);
            break;

        case MODE_ASCII:
            cputc(c);
            break;

        case MODE_ANSI:
            putchar_ansi(c);
            break;

        default:
            pcputc(c);
            break;
    }

    /*if ((rx[i] >= 0x20) && (rx[i] < 0x7e) || (rx[i] == ' ') || (rx[i] == '\r') || (rx[i] == '\n'))
    {
        //pcprintf("%c", rx[i]);  !!!!
        cputc(rx[i]);
        //sprintf(tempstring, "{lgn}[$%02x]", rx[i]);
        //pcprintf(tempstring);
    }
    else
    {
        sprintf(tempstring,"{red}[$%02x]{wht}", rx[i]);
        pcprintf(tempstring);
    }
    */
}

/* Function that is used as a call-back on socket events */
byte_t comunica (byte_t p)
{
  unsigned int i;
  unsigned char *rx=s->rx;

  asm("inc $d020");

  socket_select(s);
  switch(p) 
  {
  case WEEIP_EV_CONNECT:
    pcprintf("{clr}Connected.  (F9 to Disconnect)\n");
    
    // Send telnet GO AHEAD command
    //socket_send("\0377\0371",2);
    break;

  case WEEIP_EV_DATA:
  case WEEIP_EV_DISCONNECT_WITH_DATA:
    // Print what comes from the server
    for(i=0;i<s->rx_data;i++) 
    {
      lpoke(0x40000+byte_log,rx[i]);
      byte_log++;

      handle_rx(rx[i]);
    }
    lpoke(0x12000,(byte_log>>0)&0xff);
    lpoke(0x12001,(byte_log>>8)&0xff);
    lpoke(0x12002,(byte_log>>16)&0xff);
    lpoke(0x12003,(byte_log>>24)&0xff);

    // Fall through if its a disconnect with data
    if (p==WEEIP_EV_DATA) break;
    // FALL THROUGH
  case WEEIP_EV_DISCONNECT:      
      socket_release(s);     
      disconnected = TRUE;
      break;
  }
  
  return 0;
}

char getanykey()
{
  pcprintf("Press a key to continue...");
  return cgetc();
}

void connect_to_host(char* hostname, unsigned int port_number)
{
    IPV4 address;

    character_mode = MODE_PETSCII;
    first_char = 1;
    telnet_state = TELNET_STATE_INIT;

    sprintf(tempstring, "\nPreparing to connect to %s\n", bbs_list[nbbs].name);
    pcprintf(tempstring);

    POKE(198, 0);  // Clear keyboard buffer

    if (!dns_hostname_to_ip(hostname, &address))
    {
        sprintf(tempstring, "{red}Could not resolve hostname '%s'\n",hostname);
        pcprintf(tempstring);
        getanykey();
        return;
    }

    sprintf(tempstring,"Host '%s' resolves to %d.%d.%d.%d\n", hostname, address.b[0], address.b[1], address.b[2], address.b[3]);
    pcprintf(tempstring);

    pcprintf("\n{wht}Connecting...")
    disconnected = FALSE;

    s = socket_create(SOCKET_TCP);
    socket_set_callback(comunica);
    socket_set_rx_buffer(buf, 1500);
    socket_connect(&address, port_number);

    // Text to light green by default
    POKE(0x0286, 0x0d);

    while (1)
    {
        // XXX Actually only call it periodically
        if (PEEK(0xD7FA) != last_frame_number) 
        {
            task_periodic();
            last_frame_number = PEEK(0xD7FA);
        }

        if (disconnected)
        {
            pcprintf("\n\n{red}Disconnected!  ");
            getanykey();
            return;
        }

        // Monitor hardware accelerated keyboard input for extra C65 keys only
        if (PEEK(0xD610)) 
        {
            if (PEEK(0xD610) == KEY_F9 ) 
            {
                pcprintf("\n\n{red}Disconnecting...\n");
                socket_reset();                
            }
            POKE(0xD610, 0);
        }

        // Directly read from C64's keyboard buffer
        if (PEEK(198)) {
            lcopy(631, (long)buf, 10);
            socket_select(s);
            // Only consume keys if socket_send() succeeds
            if (socket_send(buf, PEEK(198))) {
                POKE(198, 0);
            }
        }
    }
}

void show_address_book()
{
    pcprintf("\nAddress Book:\n");

    for (nbbs = 0; bbs_list[nbbs].port_number; nbbs++)
    {
        sprintf(tempstring, "%c.%-17s ", 'a' + nbbs, bbs_list[nbbs].name);
        pcprintf(tempstring);
    }

    pcprintf("\n");
}

void show_menu()
{
    int i;

    clrscr();
    gohome();

    pcprintf("{clr}{wht}Haustierbegriff {yel}{blon}V4{bloff} {wht}by {grn}Schema{wht}/{lblu}AIC{wht}\n");

    // Get MAC address from ethernet controller
    for (i = 0; i < 6; i++) mac_local.b[i] = PEEK(0xD6E9 + i);
    sprintf(tempstring, "\n{wht}My MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n",
        mac_local.b[0], mac_local.b[1], mac_local.b[2],
        mac_local.b[3], mac_local.b[4], mac_local.b[5]);
    pcprintf(tempstring);

    if (!dhcp_configured)
    {
        // Do DHCP auto-configuration
        pcprintf("Configuring network via DHCP...\n");
        dhcp_autoconfig();

        while (!dhcp_configured)
        {
            task_periodic();
            asm("inc $d020");
        }
        bordercolor(COLOUR_DARKGREY);
    }

    sprintf(tempstring, "\nMy IP is %d.%d.%d.%d\n", ip_local.b[0], ip_local.b[1], ip_local.b[2], ip_local.b[3]);
    pcprintf(tempstring);
    //sprintf(tempstring, "Netmask  %d.%d.%d.%d\n",
    //pcprintf(tempstring);

    sprintf(tempstring, "\nScreen Mode %s (F7 to Switch)\n", screen_mode_strings[screenmode]);
    pcprintf(tempstring);

    show_address_book();
}

void change_screen_mode()
{
    screenmode++;

    switch (screenmode)
    {
        case SCREEN_80x25:
            setscreensize(80, 25);
            break;

        case SCREEN_80x50:
            setscreensize(80, 50);
            break;

        case SCREEN_40x25:
            setscreensize(40, 25);
            break;

        case SCREEN_LAST:
        default:
            screenmode = SCREEN_80x25;
            setscreensize(80, 25);
            break;
    }
}

void main(void)
{
  // Network Variables
  EUI48 mac;
  unsigned int port_number=23;
  char* hostname;

  // Local Variables
  unsigned char i;
  char ch;
  
  //srand(random32(0)); TODO - This locks?
  
  // MEGA65 Initialization ---------------------------------------------------------
  POKE(0,65);  // 40 MHz
  mega65_io_enable(); 
 
  // Screen Initialization ---------------------------------------------------------
  conioinit();
  setscreenaddr(SCREEN_BASE);
  screenmode = SCREEN_80x25;
  setscreensize(80,50);
  bordercolor(COLOUR_DARKGREY);
  bgcolor(0);
  textcolor(1);

  // Keyboard Initialization ---------------------------------------------------------
  flushkeybuf(); 
  while(PEEK(0xD610)) POKE(0xD610,0);   // Clear $D610 key buffer

  // Network Initialization ---------------------------------------------------------
  
  // Fix invalid MAC address multicast bit
  POKE(0xD6E9,PEEK(0xD6E9)&0xFE);
  // Mark MAC address as locally allocated
  POKE(0xD6E9,PEEK(0xD6E9)|0x02);

  // Setup WeeIP ---------------------------------------------------------

  weeip_init();
  task_cancel(eth_task);
  task_add(eth_task, 0, 0,"eth");
  
  // Clear buffer of received data we maintain for debugging
  lfill(0x12000,0,4);
  lfill(0x40000,0,32768);
  lfill(0x48000,0,32768);
  lfill(0x50000,0,32768);
  lfill(0x58000,0,32768);
  
  // Main Loop ---------------------------------------------------------

  while (1)
  {
      show_menu();
      ch = cgetc();

      // Debug Keys
      cputdec(ch, 4, 0);
      pcprintf("\n");

      switch (ch)
      {
          case KEY_F7:
              change_screen_mode();
              break;

          default:
              break;
      }

      if ((ch >= 97) && (ch <= 122))  //a-z
      {
          nbbs = ch - 97;
          hostname = bbs_list[nbbs].host_name;
          port_number = bbs_list[nbbs].port_number;

          connect_to_host(hostname, port_number);
          continue;
      }
  }
}
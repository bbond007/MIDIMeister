///////////////////////////////////////////////////////////////////////////////////////
//
// ---------------------------
//	2022 by BBond007
// ---------------------------
//  HPS compatible driver ALSA for MiSTer FPGA platform and mt32pi or perhaps other
//  devices.
//  Use at your own risk...

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <argp.h>
#include <signal.h>
#include <pthread.h>
#include <alsa/asoundlib.h>     /* Interface to the ALSA system */
#include <linux/serial.h>
#include <linux/ioctl.h>
#include <asm/ioctls.h>
#include "misc.h"
#include "udpsock.h"
#include "serial.h"


snd_seq_t* seq             = NULL;
char * version             = "MIDIMeister $0004h BBond007";
char * name                = "MIDIMeister";
char * serialDevice        = "/dev/ttyS1";
unsigned char DEBUG        = TRUE;
int fdSerial               = -1;
int socket_out             = -1;
#ifdef INCLUDE_UDP
unsigned int UDPServerPort = 1999;
#endif

///////////////////////////////////////////////////////////////////////////////////////
//
// int alsa_midi_setup(snd_seq_t** seq)
//
void sig_handler(int sig) 
{
    misc_all_notes_off(fdSerial, socket_out);
    misc_print(0, "Exiting program signal --> '%s'\n", strsignal(sig));
    exit(-3);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// int alsa_midi_setup(snd_seq_t** seq)
//
int alsa_midi_setup()
{
    int port_in_id;

    if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0)
    {
        fprintf(stderr, "Error opening ALSA sequencer.\n");
        close(fdSerial);
        exit(-2);
    }

    snd_seq_set_client_name(seq, name);

    if (port_in_id = snd_seq_create_simple_port(seq, "MIDI in",
                     SND_SEQ_PORT_CAP_SUBS_WRITE |
                     SND_SEQ_PORT_CAP_WRITE,
                     SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                     SND_SEQ_PORT_TYPE_MIDI_MT32 |
                     SND_SEQ_PORT_TYPE_SYNTHESIZER) < 0)
    {
        fprintf(stderr, "Error creating sequencer MIDI in port.\n");
    }

    return port_in_id;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// void handle_alsa_midi_event(snd_seq_event_t * ev)
//

void handle_alsa_midi_event(snd_seq_event_t * ev)
{
    unsigned char bytes[]   = {0x00, 0x00, 0xFF};
    unsigned int  sysex_len;
    unsigned int  channel;
    unsigned int  val;

    switch (ev->type)
    {
    case SND_SEQ_EVENT_PORT_SUBSCRIBED:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_PORT_SUBSCRIBED\n");
        #endif
        break;

    case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_PORT_UNSUBSCRIBED\n");
        #endif
        break;

    case SND_SEQ_EVENT_CLIENT_START:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_CLIENT_START\n");
        #endif
        break;

    case SND_SEQ_EVENT_TEMPO:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_TEMPO\n");
        #endif
        break;

    case SND_SEQ_EVENT_QFRAME:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_QFRAME\n");
        #endif
        break;

    case SND_SEQ_EVENT_TIMESIGN:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_TIMESIGN\n");
        #endif
        break;

    case SND_SEQ_EVENT_NOTE:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_NOTE\n");
        #endif
        break;

    case SND_SEQ_EVENT_CONTROL14:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_CONTROL14\n");
        #endif
        break;

    case SND_SEQ_EVENT_NONREGPARAM:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_NONREGPARAM\n");
        #endif
        break;

    case SND_SEQ_EVENT_REGPARAM:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_REGPARAM\n");
        #endif
        break;

    case SND_SEQ_EVENT_NOTEOFF:
        bytes[0] = 0x80 | ev->data.control.channel;
        bytes[1] = ev->data.note.note;
        bytes[2] = ev->data.note.velocity;
        #ifdef INCLUDE_SERIAL
        if(fdSerial != -1)
            write(fdSerial, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_UDP
        if (socket_out != -1)
            udpsock_write(socket_out, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_NOTEOFF      %02X   --> %02X %02X %02X\n", bytes[0]&0xF0, bytes[0]&0xF, bytes[1], bytes[2]);
        #endif
        break;

    case SND_SEQ_EVENT_NOTEON:
        bytes[0] = 0x90 | ev->data.control.channel;
        bytes[1] = ev->data.note.note;
        bytes[2] = ev->data.note.velocity;
        #ifdef INCLUDE_SERIAL
        if (fdSerial != -1)
            write(fdSerial, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_UDP
        if (socket_out != -1)
            udpsock_write(socket_out, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_NOTEON       %02X   --> %02X %02X %02X\n", bytes[0]&0xF0, bytes[0]&0xF, bytes[1], bytes[2]);
        #endif
        break;

    case SND_SEQ_EVENT_KEYPRESS:
        bytes[0] = 0xA0 | ev->data.control.channel;  // *new* was 90 in original code
        bytes[1] = ev->data.note.note;
        bytes[2] = ev->data.note.velocity;
        #ifdef INCLUDE_SERIAL
        if (fdSerial != -1)
            write(fdSerial, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_UDP
        if (socket_out != -1)
            udpsock_write(socket_out, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_KEYPRESS     %02X   --> %02X %02X %02X\n", bytes[0]&0xF0, bytes[0]&0xF, bytes[1], bytes[2]);
        #endif
        break;

    case SND_SEQ_EVENT_CONTROLLER:
        bytes[0] = 0xB0 | ev->data.control.channel;
        bytes[1] = ev->data.control.param;
        bytes[2] = ev->data.control.value;
        #ifdef INCLUDE_SERIAL
        if (fdSerial != -1)
            write(fdSerial, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_UDP
        if (socket_out != -1)
            udpsock_write(socket_out, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_CONTROLLER   %02X   --> %02X %02X %02X\n", bytes[0]&0xF0, bytes[0]&0xF, bytes[1], bytes[2]);
        #endif
        break;

    case SND_SEQ_EVENT_PGMCHANGE:
        bytes[0] = 0xC0 | ev->data.control.channel;
        bytes[1] = ev->data.control.value;
        #ifdef INCLUDE_SERIAL
        if (fdSerial != -1)
            write(fdSerial, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_UDP
        if (socket_out != -1)
            udpsock_write(socket_out, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_PGMCHANGE    %02X   --> %02X %02X %02X\n", bytes[0]&0xF0, bytes[0]&0xF, bytes[1], bytes[2]);
        #endif
        break;

    case SND_SEQ_EVENT_CHANPRESS:
        bytes[0] = 0xD0 | ev->data.control.channel;
        bytes[1] = ev->data.control.value;
        #ifdef INCLUDE_SERIAL
        if (fdSerial != -1)
            write(fdSerial, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_UDP
        if (socket_out != -1)
            udpsock_write(socket_out, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_CHANPRESS    %02X   --> %02X %02X %02X\n", bytes[0]&0xF0, bytes[0]&0xF, bytes[1], bytes[2]);
        #endif
        break;

    case SND_SEQ_EVENT_PITCHBEND:
        channel  = ev->data.control.channel;
        val      = ev->data.control.value + 0x2000;
        bytes[0] = 0xE0 | channel;
        bytes[1] = (unsigned char)(val & 0x7F);
        bytes[2] = (unsigned char)(val >> 7);
        #ifdef INCLUDE_SERIAL
        if (fdSerial != -1)
            write(fdSerial, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_UDP
        if (socket_out != -1)
            udpsock_write(socket_out, bytes, sizeof(bytes));
        #endif
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_PITCHBEND    %02X   --> %02X %04X\n", bytes[0]&0xF0, bytes[0]&0xF, val);
        #endif
        break;

    case SND_SEQ_EVENT_SYSEX:
        sysex_len = ev->data.ext.len;
        misc_print(1, "SND_SEQ_EVENT_SYSEX # = %04X   --> ", sysex_len);
        unsigned char* valPtr = (unsigned char*)ev->data.ext.ptr;
        for (int i = 0; i < sysex_len; i++)
        {
            unsigned char val = valPtr[i];
            #ifdef INCLUDE_SERIAL
            if (fdSerial != -1)
                write(fdSerial, &val, sizeof(val));
            #endif
            #ifdef INCLUDE_UDP
            if (socket_out != -1)
                udpsock_write(socket_out, &val, sizeof(val));
            #endif
            #ifdef INCLUDE_DEBUG
            misc_print(1, "%02X ", val);
            #endif
        }
        #ifdef INCLUDE_DEBUG
        misc_print(1, "\n");
        #endif
        break;

    default:
        #ifdef INCLUDE_DEBUG
        misc_print(1, "SND_SEQ_EVENT_UNKNOWN --> %d\n", ev->type);
        #endif
        break;
    }
}


///////////////////////////////////////////////////////////////////////////////////////
//
// int main(int argc, char** argv)
//
int main(int argc, char** argv)
{
    int result;
    snd_seq_event_t* seq_ev = NULL;
#ifdef INCLUDE_SERIAL
    struct termios oldtio, newtio;
    struct serial_struct ser_info;
#endif
#ifdef POLL_SEQ_HAND
    struct pollfd* pfd;
    int npfd;
#endif
#ifdef INCLUDE_UDP
    char* UDPServer;
    char ipAddr[255];
#endif
    misc_print(0, "%s\n", version);
    if (misc_check_args_option(argc, argv, "QUIET"))
        DEBUG = FALSE;

#ifdef INCLUDE_UDP
    result = misc_check_args_option(argc, argv, "UDP");
    if (result > 0)
    {
        if (result && (result + 1 < argc))
        {
            UDPServer = argv[result + 1];
            strcpy(ipAddr, UDPServer);
            misc_print(0, "UDP Server --> %s\n", ipAddr);

            if (!misc_is_ip_addr(ipAddr))
            {
                if (!misc_hostname_to_ip(ipAddr, ipAddr))
                {
                    misc_print(0, "Unable to get IP address for --> %s\n", UDPServer);
                    exit(-4);
                }
            }
            misc_print(0, "Connecting to server --> %s:%d\n", UDPServer, UDPServerPort);
            socket_out = udpsock_client_connect(ipAddr, UDPServerPort);
            if (socket_out == -1)
            {
                misc_print(0, "udpsock_client_connect failed :(\n", UDPServer);
                exit(-5);
            }
        }
    }
#endif

    misc_set_priority(TASK_PRIORITY);
    alsa_midi_setup();

#ifdef INCLUDE_SERIAL
    if (socket_out == -1)
    {
        /* save current fdSerial port settings */
        fdSerial = open(serialDevice, O_RDWR | O_NOCTTY);
        if (fdSerial < 0)
        {
            misc_print(0, "Unable to open %s :(\n", serialDevice);
            exit(-1);
        }
        tcgetattr(fdSerial, &oldtio);
        bzero(&newtio, sizeof(newtio));
        newtio.c_cflag = CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        // Raw output
        newtio.c_oflag = 0;
        //  ICANON  : enable canonical input
        // disable all echo functionality, and don't send signals to calling program
        newtio.c_lflag = 0; // non-canonical
        //  set up: we'll be reading 4 bytes at a time.
        newtio.c_cc[VTIME] = 0;     /* inter-character timer unused */
        newtio.c_cc[VMIN] = 1;      /* blocking read until n character arrives */
        // now clean the serial line and activate the settings for the port
        tcflush(fdSerial, TCIFLUSH);
        tcsetattr(fdSerial, TCSANOW, &newtio);
        serial_set_baud(serialDevice, fdSerial, 31250);
    }
#endif

#ifdef POLL_SEQ_HAND
    npfd = snd_seq_poll_descriptors_count(seq, POLLIN);
    pfd = (struct pollfd*)alloca(npfd * sizeof(struct pollfd));
    snd_seq_poll_descriptors(seq, pfd, npfd, POLLIN);
#endif

#ifdef INCLUDE_DEBUG
    misc_print(0, "DEBUG --> %s\n", DEBUG ? STR_TRUE : STR_FALSE);
#endif

    signal(SIGTERM, sig_handler); //catch kill
    signal(SIGINT, sig_handler); //catch ctrl-c
    misc_print(0, "Entering main loop...\n");
    while (TRUE)
    {
#ifdef POLL_SEQ_HAND
        if (poll(pfd, npfd, 100) > 0)
        {
#endif
            result = snd_seq_event_input(seq, &seq_ev);
            if (result >= 0)
                handle_alsa_midi_event(seq_ev);
#ifdef POLL_SEQ_HAND
        }
#endif
    }
    misc_print(0, "\n --> exiting main thread...\n");

#ifdef INCLUDE_SERIAL
    if (fdSerial != -1)
    {
        // restore the old port settings
        tcsetattr(fdSerial, TCSANOW, &oldtio);
        close(fdSerial);
    }
#endif
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>
#include <netdb.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <net/if.h>
#include "misc.h"
#include "serial.h"
#include "udpsock.h"

extern unsigned char DEBUG; 
///////////////////////////////////////////////////////////////////////////////////////
//
//  void misc_print(int priority, const char* format, ... )
//
void misc_print(int priority, const char* format, ... )
{
    if(DEBUG || priority == 0)
    {
        va_list args;
        va_start (args, format);
        vprintf (format, args);
        va_end (args);
    }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// int misc_set_priority(int priority)
//
int misc_set_priority(int priority)
{
    misc_print(0, "Setting task priority --> %d\n", priority);
    if (setpriority(PRIO_PROCESS, getpid(), priority) == 0)
        return TRUE;
    else
        misc_print(0, "ERROR: unable to set task priority --> %s\n", strerror(errno));
    return FALSE;
}


///////////////////////////////////////////////////////////////////////////////////////
//
// BOOL misc_check_args_option (int argc, char *argv[], char * option)
//
int misc_check_args_option (int argc, char *argv[], char * option)
{
    int result = FALSE;
    if(argc > 1)
        for (int i = 1; i< argc; i++)
        {
            if (strcasecmp(argv[i], option) == 0)
            {
                result = i;
                break;
            }
        }
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// void misc_all_notes_off(int fdSerial, int socket_out)
//
void misc_all_notes_off(int fdSerial, int socket_out)
{
    unsigned char bytes[3] = { 0x00, 0x00, 0x00 };
    for (unsigned char channel = 0xb0; channel <= 0xbf; channel++)
    {
        for (unsigned char i = 0; i <= 1; i++)
        {
            bytes[0] = channel;
            bytes[1] = (i == 0)?0x7b:0x40;
            #ifdef INCLUDE_DEBUG
            misc_print(1, "ALL NOTES OFF!                    --> %02X %02X %02X\n", bytes[0], bytes[1], bytes[2]);
            #endif
            #ifdef INCLUDE_SERIAL
            if (fdSerial != -1)
                write(fdSerial, bytes, sizeof(bytes));
            #endif
            #ifdef INCLUDE_UDP
            if (socket_out != -1)
                udpsock_write(socket_out, bytes, sizeof(bytes));
            #endif
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////
//
// int misc_is_ip_addr(char *ipAddr)
//
int misc_is_ip_addr(char *ipAddr)
{
    char validChr[] = ".0987654321";
    while (*ipAddr != '\0')
        if (strchr(validChr, *ipAddr++) == NULL)
            return FALSE;
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// int misc_hostname_to_ip(char * hostname , char* ip)
//
int misc_hostname_to_ip(char * hostname, char* ipAddr)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
    if ( (he = gethostbyname( hostname ) ) == NULL)
    {
        // get the host info
        misc_print(0, "ERROR : misc_hostname_to_ip()\n");
        return FALSE;
    }
    addr_list = (struct in_addr **) he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ipAddr, inet_ntoa(*addr_list[i]) );
        return TRUE;
    }
    return FALSE;
}


/*
static snd_midi_event_t * parser;
extern snd_seq_t  * seq;
static snd_seq_event_t ev;
static int portNo   = -1;
static int devNo    = -1;

///////////////////////////////////////////////////////////////////////////////////////
//
// void alsa_reset_seq_event(snd_seq_event_t * ev)
//
void misc_alsa_reset_seq_event(snd_seq_event_t * ev)
{
    snd_seq_ev_clear(ev);
    snd_seq_ev_set_direct(ev);
    snd_seq_ev_set_dest(ev, portNo, devNo);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// void alsa_send_midi_raw(char * buf, int bufLen)
//
void misc_alsa_send_midi_raw(char * buf, int bufLen)
{
#ifdef ALSA_ENCODE_BYTE
    for (int i = 0; i < bufLen; i++)
    {
        int result  = snd_midi_event_encode_byte(parser, buf[i], &ev);
        if(result == TRUE)
        {
            snd_seq_event_output(seq, &ev);
            snd_seq_drain_output(seq);
            misc_alsa_reset_seq_event(&ev);
            snd_midi_event_reset_encode(parser);
        }
    }
#else
    int sent = 0;
    while(sent < bufLen)
    {
        sent += snd_midi_event_encode(parser, &buf[sent], bufLen - sent, &ev);
        if(ev.type !=  SND_SEQ_EVENT_NONE)
        {
            snd_seq_event_output(seq, &ev);
            snd_seq_drain_output(seq);
            misc_alsa_reset_seq_event(&ev);
            snd_midi_event_reset_encode(parser);
        }
    }
#endif
}
*/

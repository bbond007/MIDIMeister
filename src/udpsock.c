#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/soundcard.h>
#include <errno.h>
#include "misc.h"
static struct sockaddr_in server_addr;

///////////////////////////////////////////////////////////////////////////////////////
//
// int udpsock_client_connect(char * ipAddr, int port)
//
int udpsock_client_connect(char* ipAddr, int port)
{
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        misc_print(0, "ERROR:socket_client_connect() --> Socket creation error\n");
        return -1;
    }
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (strlen(ipAddr) == 0)
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    else if (inet_pton(AF_INET, ipAddr, &server_addr.sin_addr) <= 0)
    {
        misc_print(0, "ERROR: udpsock_client_connect() --> Invalid IP address\n");
        return -1;
    }
    return sock;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// int udpsock_write(int sock, char * ipAddr, int port)
// 

int udpsock_write(int sock, char* buf, int bufLen)
{
    int result = sendto(sock,
        (const char*)buf,
        bufLen,
        MSG_CONFIRM,
        (const struct sockaddr*)&server_addr,
        sizeof(server_addr));
    if (result < 0)
        misc_print(1, "ERROR: udpsock_write() --> %d : %s\n", result, strerror(errno));
    return result;
}
#include <stdlib.h>
#include <string.h>
#include <asm/termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include"misc.h"

int ioctl(int fd, unsigned long request, ...);

///////////////////////////////////////////////////////////////////////////////////////
//
// int serial_set_baud(char * serialDevice, int fdSerial, int baud)
//
int serial_set_baud(char * serialDevice, int fdSerial, int baud)
{
    struct termios2 tio;
    misc_print(0, "Setting %s to %d baud.\n",serialDevice, baud);
    ioctl(fdSerial, TCGETS2, &tio);
    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = baud;
    tio.c_ospeed = baud;
    ioctl(fdSerial, TCSETS2, &tio);
    return 0;
}

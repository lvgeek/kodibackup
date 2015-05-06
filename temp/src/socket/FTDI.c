#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

/* Open File Descriptor */
int USB = open( "/dev/ttyUSB0", O_RDWR| O_NOTTY );

/* Error Handling */
if ( USB < 0 )
{
cout << "Error " << errno << " opening " << "/dev/ttyUSB0" << ": " << strerror (errno) << endl;
}

/* *** Configure Port *** */
struct termios tty;
memset (&tty, 0, sizeof tty);

/* Error Handling */
if ( tcgetattr ( USB, &tty ) != 0 )
{
cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << endl;
}

/* Set Baud Rate */
cfsetospeed (&tty, B9600);
cfsetispeed (&tty, B9600);

/* Setting other Port Stuff */
tty.c_cflag     &=  ~PARENB;        // Make 8n1
tty.c_cflag     &=  ~CSTOPB;
tty.c_cflag     &=  ~CSIZE;
tty.c_cflag     |=  CS8;
tty.c_cflag     &=  ~CRTSCTS;       // no flow control
tty.c_lflag     =   0;          // no signaling chars, no echo, no canonical processing
tty.c_oflag     =   0;                  // no remapping, no delays
tty.c_cc[VMIN]      =   0;                  // read doesn't block
tty.c_cc[VTIME]     =   5;                  // 0.5 seconds read timeout

tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines
tty.c_iflag     &=  ~(IXON | IXOFF | IXANY);// turn off s/w flow ctrl
tty.c_lflag     &=  ~(ICANON | ECHO | ECHOE | ISIG); // make raw
tty.c_oflag     &=  ~OPOST;              // make raw

/* Flush Port, then applies attributes */
tcflush( USB, TCIFLUSH );

if ( tcsetattr ( USB, TCSANOW, &tty ) != 0)
{
cout << "Error " << errno << " from tcsetattr" << endl;
}

/* *** WRITE *** */

unsigned char cmd[] = {'I', 'N', 'I', 'T', ' ', '\r', '\0'};
int n_written = write( USB, cmd, sizeof(cmd) -1 );

/* Allocate memory for read buffer */
char buf [256];
memset (&buf, '\0', sizeof buf);

/* *** READ *** */
int n = read( USB, &buf , sizeof buf );

/* Error Handling */
if (n < 0)
{
     cout << "Error reading: " << strerror(errno) << endl;
}

/* Print what I read... */
cout << "Read: " << buf << endl;

close(USB);

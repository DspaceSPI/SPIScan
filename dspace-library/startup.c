#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "../ArdInterface/ardintf.h"

#define ARDUINO_TTY	"/dev/ttyUSB0"	  // change to whatever we discover it should really be
#define GPS_TTY		"/dev/tty0"	  // change to whatever we discover it should really be

extern void startup_scan(void);
void
startup_dspace(void)
{
	int fd1, fd2;

	startup_scan();
	ard_startup();
}

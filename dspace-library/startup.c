#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "../ArdInterface/ardintf.h"

extern void startup_scan(void);
extern void gps_startup(void);
void
startup_dspace(void)
{
	int fd1, fd2;

	startup_scan();
	ard_startup();
	gps_startup();
}

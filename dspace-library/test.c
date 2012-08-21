#include <stdio.h>
extern void scan_start(int);
extern void scan_wait(void);
main()
{
	for (;;) {
		int c = getchar();
		
		if (c < 0)
			break;
		if (c >= '0' && c <= '2') {
			printf("\nstarting\n");
			scan_start(c-'0');
			printf("waiting\n");
			scan_wait();
			printf("done\n");
		}
	}
}

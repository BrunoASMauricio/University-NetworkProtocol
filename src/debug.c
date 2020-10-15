
#include "debug.h"

void dumpBin(char* buf, int size, const char *fmt,...)
{
	va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
	for(int i = 0; i < size; i++){
		fprintf(stdout, "0x%02hhx ", buf[i]);
	}
	fprintf(stdout, "\n");

	va_end(args);
	if (meta.log) {
		va_start(args, fmt);
		vfprintf(meta.log, fmt, args);
		for(int i = 0; i < size; i++){
			fprintf(meta.log, "0x%02hhx ", buf[i]);
		}
		fprintf(meta.log, "\n");
		va_end(args);
	}
}

void
printfLog(const char *fmt, ...)
{
	va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
	if (meta.log) {
		fprintf(meta.log, "[!] ");
		vfprintf(meta.log, fmt, args);
	}
	va_end(args);
}


void
printfErr(const char *fmt, ...)
{
	if (!meta.debug) return;
	va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
	if (meta.log) {
		fprintf(meta.log, "[X] ");
		vfprintf(meta.log, fmt, args);
	}
    va_end(args);
}


void
fatalErr(const char *fmt, ...)
{
	FILE* t;
	va_list args;
	if ((t = fopen("./fatal", "w")) != NULL) {
		va_start(args, fmt);
		vfprintf(t, fmt, args);
		fflush(t);
		va_end(args);
	} else {
		fprintf(stderr, "Could not create \"fatal\" file\n");
	}
	va_start(args, fmt);
	fprintf(stderr, "[XX]");
    vfprintf(stderr, fmt, args);
	fflush(stderr);
    va_end(args);
	exit(EXIT_FAILURE);
}

void
testAll(){
	char a[6];
	a[0] = 0xaf;
	a[1] = 0x99;
	a[2] = 0x82;
	a[3] = 0xff;
	a[4] = 0x00;
	a[5] = 0x11;
	printf("Starting protocol test\n---------\n");

	printf("Testing printfLog. Expected output:\n");
	printf(">>1 teste 0.5\n");
	printf(">>%d teste %0.1f\n", 1, 0.5);

	printf("Testing dumpBin. Expected output:\n");
	printf(">>Hello 4: 0xaf 0x99 0x82 0xff 0x00 0x11\n");
	
	dumpBin(a, sizeof(a), ">>Hello %d: ", 4);





	printf("Ending protocol test\n---------\n");

}

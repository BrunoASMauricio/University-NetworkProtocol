#ifndef UTILS_DEBUG
#define UTILS_DEBUG

#include <stdarg.h>

void dumpBin(char* buf, int size, const char *fmt,...);

void printMessage(void* buff, int size);

char getThreadChar();
	
void printfLog(const char *fmt, ...);

void fatalErr(const char *fmt, ...);

#endif

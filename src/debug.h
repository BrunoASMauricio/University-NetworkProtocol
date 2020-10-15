#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>

#define printf printfLog

void
/*
 * print a message, and dump a binary
 */
dumpBin(char* buf, int size, const char *fmt,...);

void
/*
 * Normal output
 */
printfLog(const char *fmt, ...);

void
/*
 * Error output
 */
printfErr(const char *fmt, ...);

void
/*
 * Try to print to a file
 * Print to stderr
 * End program with EXIT_FAILURE
 */
fatalfErr(const char *fmt, ...);

void
/*
 * Performs a Self Test
 * When a problem can be detected, will halt the program
 * and return an error
 */
testAll();


#endif

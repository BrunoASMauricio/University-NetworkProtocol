#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>


void
/*
 * print a message, and dump a binary
 */
dumpBin(char* buf, int size, const char *fmt,...);

void
/*
 * Provides output with variables
 */
printfLog(const char *fmt, ...);

void
/*
 * Simple output
 * If possible, this one should be merged with printfLog
 */
printLog(const char *msg);

void
/*
 * Error output with variables
 */
printfErr(const char *fmt, ...);

void
/*
 * Simple error output
 * If possible, this one should be merged with printfErr
 */
printErr(const char *fmt);

void
/*
 * Performs a Self Test
 * When a problem can be detected, will halt the program
 * and return an error
 */
testAll();


#endif

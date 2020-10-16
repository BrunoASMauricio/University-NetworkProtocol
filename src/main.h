#ifndef MAIN_H
#define MAIN_H


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>


#include "data.c"
#include "debug.c"
#include "protocol.c"
#include "RX.c"
#include "TX.c"


void
/*
 * Performs node setup
 */
setup();

void
/*
 * Continuously handles the received packets
 */
handler();

void
/*
 * Clean the data structures
 * Save whatever data needs saving
 */
clean();




#endif

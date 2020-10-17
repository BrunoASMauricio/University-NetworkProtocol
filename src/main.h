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


/*
 * Performs node setup
 */
void
setup();

/*
 * Continuously handles the received packets
 */
void
handler();


/*
 * Clean the data structures
 * Save whatever data needs saving
 */
void
clean();




#endif

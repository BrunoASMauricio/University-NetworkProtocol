#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <netdb.h>
#include <stddef.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef uint8_t byte;


/*
 * Performs a network test
 * sets Self.isMaster to the best guessed value
 * (has an internet connection)
 * TODO: real check is to check HW connection
 */
void
isMaster();


void
handleSD(SD_p* message);

void
handlePB(PB_p* message);

void
handlePR(PR_p* message);



/*snippet code provided by anand choubey, consulted on: 
    https://www.codeproject.com/Articles/35103/Convert-MAC-Address-String-into-Bytes
* converts mac address string to bytes (does hex to bytes conversion) 
*/
unsigned char* 
ConverMacAddressStringIntoByte(const char *pszMACAddress, unsigned char* pbyAddress);

byte* 
getIP();

#endif

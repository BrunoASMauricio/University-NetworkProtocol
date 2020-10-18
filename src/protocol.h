#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <netdb.h>

/*
 * Performs a network test
 * Returns true if this node is the master
 * (has an internet connection)
 * false otherwise
 * not yet completed, lacks "Unset" to update self
 */
bool
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

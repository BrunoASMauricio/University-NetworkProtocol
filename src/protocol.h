#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <netdb.h>
#include <ctype.h>


typedef uint8_t byte;


/*
 * Performs a network test
 * sets Self.isMaster to the best guessed value
 * (has an internet connection)
 * TODO: real check is to check HW connection
 */
void
setMaster();

/*
 * Returns the size of the packet in buf
 * Returns -1 on undefine message type/wrong version
 */
int
getPacketSize(void* buf);

void
handleSD(void* message);

void
handlePB(void* message);

void
handlePR(void* message);



/*snippet code provided by anand choubey, consulted on: 
    https://www.codeproject.com/Articles/35103/Convert-MAC-Address-String-into-Bytes
* converts mac address string to bytes (does hex to bytes conversion) 
*/
unsigned char* 
ConverMacAddressStringIntoByte(const char *pszMACAddress, unsigned char* pbyAddress);

byte* 
getIP();

#endif

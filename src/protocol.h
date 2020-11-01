#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <netdb.h>
#include <ctype.h>


// Check for retransmission delay in us
#define DEFAULT_RETRANSMIT_CHECK 10

// Retransmission delays in ns
#define RETRANSMISSION_DELAY_TB 100
#define RETRANSMISSION_DELAY_PR 100
#define RETRANSMISSION_DELAY_NE 100
#define RETRANSMISSION_DELAY_NER 100

void
/*
 * Starts the retransmission of a certain message
 * The retransmitable field is the message type.
 * The enum specifies: rTB, rPR, rNE and rNER
 */
startRetransmission(retransmitable message_type);

void
/*
 * Halts the retransmission of a certain message
 */
stopRetransmission(retransmitable message_type);


void*
/*
* Infers from retransmitables the required retransmissions.
* Uses time_* to check if the retransmission should occur now
* sleeps for the difference in the closest timestamp and the current time
* (overshoots should be avoided, but aren't harmful)
*/
retransmit(void* dummy);

void
/*
* Re/Generates the retransmission TimeTable
* If the "deadline" is reached, generates it and sets up retransmission
*/
generateTB();

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

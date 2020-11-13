#ifndef TX_H
#include <sys/types.h> 
#include <netinet/in.h> 
#include "data.h"
#include "udp.h"
#include "protocol.h" 
#include "main.h"

#define PORTHW     8080 

#define TIMESTAMPLENGTH     2   // 2 bytes - timestamp length
#define SAMPLELENGTH        2   // 2 bytes - each sample length
#define IPLENGTH            2   // 2 bytes - ip length
#define TOTALSAMPLELENGTH IPLENGTH+SAMPLELENGTH+TIMESTAMPLENGTH // total length
#define NUMSAMPLES          10  // this value can still be changed - number of samples in each 
                                //                                  package taken from the internal queue

#define TX_H

// The transmission delay since the message is sent to WF,
// to when WF finishes sending it
// In nanoseconds
#define TRANSMISSION_DELAY 1000 // 1 us

/*
 * Interface with the WF team (TX)
 * Sends packets in the queue that are ready
 */
void*
WF_dispatcher(void* dummy);


/*
 *
 */
void*
HW_dispatcher(void* dummy);

/*
 * Place a packet in the outbound queue
 */
void
sendMessage(void* msg);


/*
 * Handles a SD message
 */
void SD_TX(int Sample_Ammount);

/*
 * Creates a PB message and adds to outbound queue 
 */
void PB_TX();

/*
 * Creates a PR message and adds to outbound queue 
 */
void PR_TX(byte Originator_IP[2], byte PBID[2], byte SNR);

/*
 * Creates a PC message and adds to outbound queue 
 */
void PC_TX(byte Reached_IP[2], byte PBID[2], byte SNR);

/*
 * Handles a TA message
 */
void TA_TX(byte Originator_IP[2], byte PBID[2]);

/*
 * Handles a TB message
 */
void TB_TX(byte PBID[2], timetable* tm, in_message* message);

/*
 * Handles an NE message
 */
void NE_TX(byte Proxy_IP[2]);

/*
 * Handles an NEP message
 */
void NEP_TX(byte Outsiders_IP[2]);

/*
 * Handles an NER message
 */
void NER_TX(byte Outsiders_IP[2]);

/*
 * Handles an NEA message
 */
void NEA_TX(byte Outsiders_IP[2], byte PBID[2]);

#endif

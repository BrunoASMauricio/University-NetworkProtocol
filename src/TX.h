#ifndef TX_H
#include <sys/types.h> 
#include <netinet/in.h> 
#include "data.h"
#include "udp.h"
#include "protocol.h" 
#include "main.h"

#define PORTHW     8080 

#define TAMTIMESTAMP    2   // 2 bytes the timestamp
#define TAMSAMPLE   2   // 2 bytes the sample of WS
#define TAMIP   2   //  2 bytes the ip
#define TAMTOTALSAMPLE TAMIP+TAMSAMPLE+TAMTIMESTAMP //TOTAL TIME
#define NUMSAMPLES 10 //could be alterated

#define TX_H

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
void SD_TX(int Sample_Ammount, void* Samples);

/*
 * Handles a PB message
 */
void PB_TX(byte PBID[2]);

/*
 * Handles a PR message
 */
void PR_TX(byte Originator_IP[2], byte PBID[2], byte SNR);

/*
 * Handles a PC message
 */
void PC_TX(byte Reached_IP, byte PBID[2], byte SNR);

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

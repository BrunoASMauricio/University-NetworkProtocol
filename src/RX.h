#ifndef RX_H
#include <sys/types.h> 
#include <netinet/in.h> 
#include "data.h"
#include "udp.h"
#include "protocol.h" 
#include "main.h"
#include <time.h>

#define PORTWS    8080
#define TAMIP   2   //  2 bytes the ip
#define DATAPAYLOAD 3 // bytes
#define TAMTOTALSAMPLE  TAMIP + DATAPAYLOAD   // bytes   


#define RX_H

/*
 * Continuously listens for new packets and injects them
 * into the InboundQueue as in_message
 * TODO
 * * Get SNR from WF
 * * Test thoroughly
 */
void*
WF_listener(void* dummy);

/*
 * Interface with the WS team (RX)
 */
void*
WS_listener(void* dummy);


/*
 * Blocks until a new packet is available in the inbound queue
 * Should release the CPU while blocking
 */
in_message*
getMessage();


/*
 * Handles a SD message
 */
void SD_RX(in_message* msg);

/*
 * Handles a PB message
 */
void PB_RX(in_message* msg);

/*
 * Handles a PR message
 */
void PR_RX(in_message* msg);

/*
 * Handles a PC message
 */
void PC_RX(in_message* msg);

/*
 * Handles a TA message
 */
void TA_RX(in_message* msg);

/*
 * Handles a TB message
 */
void TB_RX(in_message* msg);

/*
 * Handles an NE message
 */
void NE_RX(in_message* msg);

/*
 * Handles an NEP message, takes a NEP in_message from NEP_TX
 * IMPORTANT: Assumes msg->buf to have NEP format;
 * Cancels NE Retransmission and updates Node.Status to OutsideSlave
 */
void NEP_RX(in_message* msg);

/*
 * Handles an NER message
 */
void NER_RX(in_message* msg);

/*
 * Handles an NEA message
 */
void NEA_RX(in_message* msg);

#endif


#ifndef TX_H
#define TX_H

void*
/*
 * Interface with the WF team (TX)
 * Sends packets in the queue that are ready
 */
WF_dispatcher(void* dummy);


void*
/*
 *
 */
HW_dispatcher(void* dummy);

void
/*
 * Place a packet in the outbound queue
 */
sendMessage(void* msg);


#endif

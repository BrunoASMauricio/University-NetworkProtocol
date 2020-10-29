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


#endif

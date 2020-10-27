#ifndef TX_H
#include <sys/types.h> 
#include <netinet/in.h> 
#include "data.h"
#include "udp.h"
#include "protocol.h" 
#include "main.h"

#define PORTHW     8080 
#define MAXLINE 1024 
#define MAXBITSIP 16 //bits 
#define MAXSAMPLES 10  
#define MAXBITSSAMPLE 16 // bits

// LEN IN BITS
#define INITIP  8
#define LENIP   16
#define INITSA  48
#define LENSA   8
#define INITS   56
#define LENS    16

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

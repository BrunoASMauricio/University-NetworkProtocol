#ifndef TX_ROUTING
#define TX_ROUTING


/*
 * Creates a PB message and adds to outbound queue
 */
void PB_TX(void* packet);

/*
 * Creates a PR message and adds to outbound queue
 */
void PR_TX(void* PRPacket);

/*
 * Creates a PC message and adds to outbound queue 
 */
void PC_TX(byte Reached_IP[2], byte PBID[2], float PBE);


#endif

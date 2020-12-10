#ifndef TX_NETWORK
#define TX_NETWORK

/*
 * Handles an NE message
 */
void NE_TX(void* message);

/*
 * Builds and queues a NEP message, using received OutsidersIP and 
 * Self.IP for senders IP
 */
void NEP_TX(byte Outsiders_IP[2]);

/*
 * Handles an NER message
 */
void NER_TX(void* message);

/*
 * Handles an NEA message
 */
void NEA_TX(byte Outsiders_IP[2], pbid PBID);


#endif

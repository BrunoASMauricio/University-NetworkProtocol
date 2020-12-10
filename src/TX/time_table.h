#ifndef TX_TIME_TABLE
#define TX_TIME_TABLE


/*
 * Handles a TA message
 */
void TA_TX(byte Originator_IP[2], byte PBID[2]);

/*
 * Handles a TB message
 */
void TB_TX(void* message);

bool beginTBTransmission();

#endif

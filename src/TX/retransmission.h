#ifndef TX_RETRANSMISSION
#define TX_RETRANSMISSION

#define PB_TIMEOUT range(RETRANSMISSION_DELAY_PB_MIN, RETRANSMISSION_DELAY_PB_MAX)
#define TB_TIMEOUT range(RETRANSMISSION_DELAY_TB_MIN, RETRANSMISSION_DELAY_TB_MAX)
#define PR_TIMEOUT range(RETRANSMISSION_DELAY_PR_MIN, RETRANSMISSION_DELAY_PR_MAX)
#define NE_TIMEOUT range(RETRANSMISSION_DELAY_NE_MIN, RETRANSMISSION_DELAY_NE_MAX)
#define NER_TIMEOUT range(RETRANSMISSION_DELAY_NER_MIN, RETRANSMISSION_DELAY_NER_MAX)

void
/*
 * Starts the retransmission of a certain message
 * The retransmitable field is the message type.
 * The enum specifies: rTB, rPR, rNE and rNER
 * The msg argument is the message to be retransmitted
 */
startRetransmission(retransmitable message_type, void* msg);

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

#endif

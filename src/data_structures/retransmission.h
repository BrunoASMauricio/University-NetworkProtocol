#ifndef DATA_STRUCTURES_RETRANSMISSION
#define DATA_STRUCTURES_RETRANSMISSION

/*
 * The retransmitable messages
 */
enum retransmitable{
	rPB = 1,
	rTB,
	rPR,
	rNE,
	rNER
};

/*
 * Struct that helps control message
 * retransmission
*/
typedef struct{
	pthread_mutex_t Lock;
	byte Retransmitables;		// The retransmission bitmap

	unsigned long int Time_PB;
	void* PB_ret_msg;
	byte PB_ret_amm;

	unsigned long int Time_TB;
	void* TB_ret_msg;
	byte TB_ret_amm;

	unsigned long int Time_PR;
	void* PR_ret_msg;
	byte PR_ret_amm;

	unsigned long int Time_NE;
	void* NE_ret_msg;
	byte NE_ret_amm;
	
	unsigned long int Time_NER;
	void* NER_ret_msg;
	byte NER_ret_amm;
}retransmission;

#endif

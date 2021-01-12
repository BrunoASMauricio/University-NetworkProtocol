#ifndef DATA_STRUCTURES_PROTOCOL
#define DATA_STRUCTURES_PROTOCOL

#define PROTOCOL_VERSION 2

//								Messages

// Packet sizes in bytes
// (first -1 is to align the packets type with array index)
const int Packet_Sizes[11] = {-1, 56/8, 56/8, 104/8, 88/8, 144/8, 56/8, 40/8, 40/8, 40/8, 40/8};

enum packet_type{
	SD = 1,
	PB,
	PR,
	PC,
	TB,
	TA,
	NE,
	NEP,
	NER,
	NEA
};

// Maximum SD payload size in bytes
#define MAX_PAYLOAD_SIZE 256

// 256 node network TB
#define MAXIMUM_PACKET_SIZE 4496

// Check for retransmission delay in ns
#define DEFAULT_RETRANSMIT_CHECK 1E9	// 1s


//								TimeTable
#define DEFAULT_VALIDITY_DELAY 1000	//in us
#define DEFAULT_TIMESLOT_SIZE 200		//in ms

// The delay since the TB transmission is
// requested, and it first begins (in ns)
#define TB_GENERATION_DELAY	1000000	//(1ms)

//								Retransmission

// Retransmission delays in ns
#define RETRANSMISSION_DELAY_PB_MIN 3*1E9		// 3 s
#define RETRANSMISSION_DELAY_PB_MAX 5*1E9		// 5 s

#define RETRANSMISSION_DELAY_TB_MIN 6*1E9		// 1 s
#define RETRANSMISSION_DELAY_TB_MAX 7*1E9		// 2 s

#define RETRANSMISSION_DELAY_PR_MIN 1*1E9		// 1 s
#define RETRANSMISSION_DELAY_PR_MAX 2*1E9		// 2 s

#define RETRANSMISSION_DELAY_NE_MIN 2*1E9		// 2 s
#define RETRANSMISSION_DELAY_NE_MAX 3*1E9		// 3 s

#define RETRANSMISSION_DELAY_NER_MIN 2*1E9	// 2 s
#define RETRANSMISSION_DELAY_NER_MAX 3*1E9	// 3 s

#define RETRANSMISSION_ATTEMPTS_NE 5

#define PB_TIMEOUT range(RETRANSMISSION_DELAY_PB_MIN, RETRANSMISSION_DELAY_PB_MAX)
#define TB_TIMEOUT range(RETRANSMISSION_DELAY_TB_MIN, RETRANSMISSION_DELAY_TB_MAX)
#define PR_TIMEOUT range(RETRANSMISSION_DELAY_PR_MIN, RETRANSMISSION_DELAY_PR_MAX)
#define NE_TIMEOUT range(RETRANSMISSION_DELAY_NE_MIN, RETRANSMISSION_DELAY_NE_MAX)
#define NER_TIMEOUT range(RETRANSMISSION_DELAY_NER_MIN, RETRANSMISSION_DELAY_NER_MAX)


// Time the TX waits for a message when none is
// available (in us)
#define TX_MESSAGE_WAIT 1E3		//1ms

// The transmission delay since the message is sent to WF,
// to when WF finishes sending it
// In nanoseconds
#define TRANSMISSION_DELAY ((unsigned long int)(3*1E4)) // 30us/bit

// Transmission jitter in ns
#define TRANSMISSION_JITTER ((unsigned long int)(30*1E6)) // 30ms

#define DATAPAYLOADLENGTH   136
#define IPLENGTH            2
#define TOTALLENGTH IPLENGTH+DATAPAYLOADLENGTH

// Queue read delay in HW dispatcher in us
#define HW_DISPATCHER_SLEEP 0.1E5	// 0.01s

#define UNREACHABLE 65535

#define TAMIP   2   //  2 bytes the ip
#define DATAPAYLOAD 136 // bytes
#define TAMTOTALSAMPLE  TAMIP + DATAPAYLOAD   // bytes   



#endif

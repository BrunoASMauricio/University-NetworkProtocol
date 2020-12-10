#ifndef DATA_STRUCTURES_PROTOCOL
#define DATA_STRUCTURES_PROTOCOL

// Maximum SD payload size in bytes
#define MAX_PAYLOAD_SIZE 256

// Check for retransmission delay in ns
#define DEFAULT_RETRANSMIT_CHECK 1E9	// 1s

// Retransmission delays in ns
#define RETRANSMISSION_DELAY_PB 5*1E9		// 5 s
#define RETRANSMISSION_DELAY_TB 2*1E9		// 2 s
#define RETRANSMISSION_DELAY_PR 2*1E9		// 2 s
#define RETRANSMISSION_DELAY_NE 2*1E9		// 2 s
#define RETRANSMISSION_DELAY_NER 2*1E9	// 2 s

#define RETRANSMISSION_ATTEMPTS_NE 5

// Time the TX waits for a message when none is
// available (in us)
#define TX_MESSAGE_WAIT 1E3		//1ms

// The transmission delay since the message is sent to WF,
// to when WF finishes sending it
// In nanoseconds
#define TRANSMISSION_DELAY 1000 // 1 us

#define DATAPAYLOADLENGTH   3
#define IPLENGTH            2
#define TOTALLENGTH IPLENGTH+DATAPAYLOADLENGTH

// Queue read delay in HW dispatcher in us
#define HW_DISPATCHER_SLEEP 0.5E6	// 0.5s

#define UNREACHABLE 65535

#define TAMIP   2   //  2 bytes the ip
#define DATAPAYLOAD 3 // bytes
#define TAMTOTALSAMPLE  TAMIP + DATAPAYLOAD   // bytes   

#endif

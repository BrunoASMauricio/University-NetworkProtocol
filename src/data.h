#ifndef DATA
#define DATA

#include <stdint.h>
#include "main.h"
#include "debug.h"
#include "udp.h"
#include "routing_table.h"

#define UNSET 255
#define SAMPLE_SIZE 16
#define PROTOCOL_VERSION 2
#define MAXIMUM_PACKET_SIZE 4496	// 256 node network TB
#define DEFAULT_VALIDITY_DELAY 1000	//in ns
#define DEFAULT_TIMESLOT_SIZE 1		//in ms

// The delay since the TB transmission is
// requested, and it first begins (in ns)
#define TB_GENERATION_DELAY	1000000	//(1ms)

#define DEFAULT_HW_PORT 901
#define DEFAULT_WS_PORT 902
#define DEFAULT_WF_TX_PORT 903
#define DEFAULT_WF_RX_PORT 904


typedef uint8_t byte;
typedef uint16_t pbid;

/*
 * Data structures
 */

// DO NOT TOUCH THIS STRUCT

typedef struct{
	pthread_t WF_listener_t;
	pthread_t WF_dispatcher_t;
	pthread_t WS_listener_t;
	pthread_t HW_dispatcher_t;
	pthread_t Retransmission_t;
	pthread_t Main_t;

	socket_s* WF_RX;
	socket_s* WF_TX;
	bool Post;
	bool Debug;
	bool Quiet;
	FILE* Log;
	
	int WF_TX_port;
	int WF_RX_port;
	int HW_port;
	int WS_port;
} meta_data;

// TOUCH THESE :)
// Packet sizes in bytes
const int Packet_Sizes[11] = {-1, 56/8, 56/8, 80/8, 64/8, 144/8, 56/8, 40/8, 40/8, 40/8, 40/8};

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

enum node_status{
	Outside,
	Waiting,
	Inside,
	NA, //to use in the master one
};

/*
 * Holds a message to send
 */
typedef struct{
	int size;				// Buffer allocated size/total message size
	void* buf;				// Buffer where the message is stored
} out_message;
/*
 * Holds a received message
 */
typedef struct{
	int size;				// Buffer allocated size/total message size
	void* buf;				// Buffer where the message is stored
	long int received_time;	// input timestamp
	short SNR;				// link quality (perhaps not SNR)
} in_message;

typedef struct{
	long int Data;
}sample;

typedef struct{
	pthread_mutex_t lock;
	byte PBID[2];
	byte** IPs;
	int IP_amm;
	void* bitmap;
	byte bitmap_size;
	unsigned long int sync_timestamp;
	short validity_delay;
} timetable_msg;

// All 64 bits to avoid operation mistakes
// Most of the operations would be 64 bits either way
typedef struct{
	pthread_mutex_t Lock;
	unsigned long int timeslot_size;	//timeslot size (1 byte padded)
	unsigned long int table_size;		//timetable size (2 bytes padded)
	unsigned long int  local_slot;			//local slot (0 to N(umber of nodes)) (1 byte padded)
	unsigned long int sync;
} timetable;


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
 * Queue related data types
 */

typedef struct{
	void* Packet;
	void* NextEl;
	int PacketSize;
	int Pr;
} queue_el;

typedef struct{
	queue_el* First;
	queue_el* Last;
	pthread_mutex_t Lock;
	int Size;
} queue;

/*
 * List related data types
 */

typedef struct{
	void* Next;
	void* Buff;
}List_el;

typedef struct{
	List_el* First;
	List_el* Last;
	int Size;
}List;

typedef struct{
	pthread_mutex_t Lock;
	List* L;
}IPList;



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


/*
 * Structs for pbid-ip pairs table
 */
typedef struct pbid_ip_pairs{
    byte PresentIP[2];
    byte PresentPBID[2];
    struct pbid_ip_pairs* next_pair;
		bool IsLastPair;
} pbid_ip_pairs;

typedef struct{
		pbid_ip_pairs* first_pair;
		pthread_mutex_t Lock;
} pbid_ip_table;


/*
 * Internal queue is only handled by the WS and HW interfaces
 */
typedef struct{
	byte IsMaster;
	queue* OutboundQueue;
	queue* InboundQueue;
	queue* InternalQueue;
	byte IP[2];
	table* Table;
	timetable* TimeTable;
	pbid_ip_table* PBID_IP_TA;
	bool SyncTimestamp;
	retransmission Rt;
	IPList* SubSlaves;
	IPList* OutsideSlaves;
	IPList* RegisteredSlaves;
	IPList* OutsidePending;
	byte TB_PBID[2];
	node_status Status;
	//pbid_ip_pairs* RoutingPBIDTable;
    pbid PBID;
	// ...
} node;

/*
 * Returns an initialized queue
 */
queue*
newQueue();

/*
 * Stores the buffer POINTER value "Buffer", and the size "Size"
 * in the queue "Q" according to the priority "Pr".
 * DOES NOT COPY THE DATA IN THE BUFFER, STORES THE RAW POINTER
 */
void
addToQueue(void* Buffer, int Size, queue* Q, int Pr);

/*
 * Pop the next queued packet.
 * Places the size of the packet in *Size
 * Returns the buffers memmory location
 */
void*
popFromQueue(int* Size, queue* Q);

/*
 * Deallocates a queue
 */
void
delQueue(queue* Q);

/*
 * Generate a new message structure from
 * buffer
 */
in_message*
newInMessage(int size, void* buffer, timespec res);

/*
 * Clean a message structure
 */
void
delInMessage(in_message* Message);

/*
 * Generate a new message structure from
 * buffer
 */
out_message*
newOutMessage(int size, void* buffer);

/*
 * Clean a message structure
 */
void
delOutMessage(out_message* Message);

List* newList();

void delList(List* L);


void
/*
 * Inserts a new element with the stored buffer, at position position
 * in the list
 * If the position is negative, inserts in the beggining of the list
 * If the position is equal or higher than the list size, in the end
 */
insertInList(List* L, void* buffer, int position);

void
/*
 * Prints the contents in the buffers of a list
 * Assumes they are '\0' terminated
 */
printList(List* L);

void*
/*
 * Removes element at the given position
 * Returns removed element buffer or NULL if position
 * is negative or higher than list size
 */
removeFromList(List* L, int position);

IPList*
/*
 * Returns an IP List
 * IP List operations are THREAD SAFE
 */
newIPList();

void
/*
 * Destroys an IP List
 * THREAD SAFE
 */
delIPList(IPList* IPL);

bool
/*
 * Returns true if the IP is in the given IP list,
 * false otherwise
 * THREAD SAFE
 */
getIPFromList(IPList* IPL, byte IP[2]);

void
/*
 * Insert an IP into an IP List
 * Only does so if it doesn't already exist
 * THREAD SAFE
 */
insertIPList(IPList* IPL, byte IP[2]);

void
/*
 * Remove an IP from an IP List
 * THREAD SAFE
 */
removeIPList(IPList* IPL, byte IP[2]);



pbid_ip_table* pbidInitializeTable();
/*
 * intializes the PBID-IP pairs table as a linked list
 * values of first PBID-IP pair are set to 0
 * returns a pointer to head of table
 *
 * head of table holds pointer to first PBID-IP pair
 * a PBID-IP pair is constituted by a PBID entry and an IP entry
 */



void pbidInsertPair(byte* IP_ofPair, byte* PBID_ofPair, pbid_ip_table* table_head);
/*
 * stores a PBID-IP pair ("IP_ofPair" and "PBID_ofPair") on the table pointed by "table_head"
 *
 * to do so, it first checks if the IP entry of pair to be added already exists:
 *   if yes - switchs only its corresponding PBID entry (discards previous PBID);
 *   if no - adds an entirely new pair
 */



int pbidSearchPair(byte* IP_ofPair, byte* PBID_ofPair, pbid_ip_table* table_head);
/*
 * should always be performed BEFORE pbid_storePair as stated on the protocol
 * returns 1 if the pair of "IP_ofPair" and "PBID_ofPair" is found on the table pointed by "table_head",
 * 				0 otherwise
 *
 * the search is made by checking only the PBID entrys as this is an unique
 * identifier, meaning that in case of a PBID match, there's forcibly a match
 * between the pairs' corresponding IPs
 */



void pbidPrintTable(pbid_ip_table* table_head);
/*
 * prints all elements of the table pointed by "table_head"
 */



void pbidRemovePair(byte* IP_toRemove, pbid_ip_table* table_head);
/*
 * removes pair with "IP_toRemove" from the table pointed by "table_head"
 */

/*
 * Builds NEP Type Message using newOutMessage()
 */ 
out_message*
buildNEPMessage(byte* SenderIP, byte* OutsiderIP);

out_message* 
buildTAMessage(byte* Originator_IP, byte * PBID);


meta_data Meta;
node Self;
byte  *ip;

#endif

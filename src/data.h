
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

typedef uint8_t byte;

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

	socket_s* Input_socket;
	socket_s* Output_socket;
	bool Post;
	bool Debug;
	FILE* Log;
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

/*
 * The retransmitable messages
 */
enum retransmitable{
	rTB = 1,
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
typedef struct{
	pthread_mutex_t Lock;
	byte PBID[2];
	IPList* IPs;
	void* Bitmap;
	byte Bitmap_size;
	long int Sync_timestamp;
	short Validity_delay;
} timetable_msg;
*/

typedef struct{
	pthread_mutex_t Lock;
	byte Timeslot_size;
	short Table_size;
	byte Local_slot;
} timetable;

/*
 * Struct that helps control message
 * retransmission
 */
typedef struct{
	pthread_mutex_t Lock;
	//timetable_msg* Tm;
	byte Retransmitables;		// The retransmission bitmap
	unsigned long int Time_TB;
	unsigned long int Time_PR;
	unsigned long int Time_NE;
	unsigned long int Time_NER;
}retransmission;

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
	retransmission Rt;
	IPList* SubSlaves;
	IPList* OutsideSlaves;
	byte TB_PBID[2];
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



meta_data Meta;
node Self;

#endif

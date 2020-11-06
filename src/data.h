
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

	socket_s* Input_socket;
	socket_s* Output_socket;
	bool Post;
	bool Debug;
	FILE* Log;
} meta_data;

// TOUCH THESE :)

const int Packet_Sizes[11] = {-1, 56, 56, 80, 64, 144, 56, 40, 40, 40, 40};

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
	long int sync_timestamp;
	short validity_delay;
} timetable_msg;

typedef struct{
	byte timeslot_size;
	short table_size;
	byte local_slots;
} timetable;


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
 * Internal queue is only handled by the WS and HW interfaces
 */
typedef struct{
	byte IsMaster;
	queue* OutboundQueue;
	queue* InboundQueue;
	queue* InternalQueue;
	byte IP[2];
	table* Table;
	short RoutingPBID;
	node_status Status;
	//pbid_ip_pairs* PB_PBIDTable;
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

meta_data Meta;
node Self;

#endif

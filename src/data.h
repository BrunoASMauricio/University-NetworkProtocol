#ifndef DATA
#define DATA

#include <stdint.h>
#include "main.h"
#include "debug.h"

#define UNSET 255
#define DEFAULT_QUEUE_SIZE

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
	bool Post;
	bool Debug;
	FILE* Log;
} meta_data;

// TOUCH THESE :)

enum packet_type{
	SD = 1,
	PB,
	PR
};

typedef struct{
	long int Data;
}sample;

typedef struct{
	byte Type;
	byte SeqNmb;
	byte TTL;
	byte SourceIP[2];
	byte NextHopIP[2];
	byte SampleAmm;
	sample* Samples;
}SD_p;

typedef struct{
	byte Type;
	byte PBID[2];
	byte OrigIP[2];
}PB_p;

typedef struct{
	byte Type;
	byte PBID[2];
	unsigned short Delay;
	byte SourceIP[2];
	byte NextHopIP[2];
}PR_p;


/*
 * Queue related data types
 */
enum priority{
	Normal = 0,
	Sync
};


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


typedef struct{
	byte IsMaster;
	queue* OutboundQueue;
	queue* InboundQueue;
	byte IP[2];
	// ...
} node;

/*
 * Returns an initialized queue
 */
queue*
newQueue();

/*
 * Inserts the packet pointed to by "packet", of type "type"
 * into the queue "q" according to the priority "pr"
 */
void
addToQueue(void* Packet, int Type, queue Q, int Pr);

/*
 * Pop the next queued packet.
 * Places the packet buffer location in the *buffer pointer
 * Returns the size of the packet
 */
int
popFromQueue(void** buffer, queue* q);

/*
 * Deallocates a queue
 */
void
delQueue(queue* Q);

meta_data Meta;
node Self;

#endif

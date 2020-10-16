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
	bool post;
	bool debug;
	FILE* log;
}metadata;

// TOUCH THESE :)

enum packet_type{
	SD = 1,
	PB,
	PR
};

typedef struct{
	long int data;
}sample;

typedef struct{
	byte type;
	byte seq_nmb;
	byte TTL;
	byte source_IP[2];
	byte next_hop_IP[2];
	byte sample_amm;
	sample* samples;
}SD_p;

typedef struct{
	byte type;
	byte PBID[2];
	byte Orig_IP[2];
}PB_p;

typedef struct{
	byte type;
	byte PBID[2];
	unsigned short delay;
	byte source_IP[2];
	byte next_hop_IP[2];
}PR_p;


/*
 * Queue related data types
 */
enum priority{
	Normal = 0,
	Sync
};


typedef struct{
	void* packet;
	void* next_el;
	int packet_size;
	int pr;
}queue_el;

typedef struct{
	queue_el* first;
	queue_el* last;
	pthread_mutex_t lock;
	int size;
}queue;


typedef struct{
	byte is_master;
	queue* outbound_q;
	queue* inbound_q;
	// ...
}node;

queue*
/*
 * Returns an initialized queue
 */
newQueue();

void
/*
 * Inserts the packet pointed to by "packet", of type "type"
 * into the queue "q" according to the priority "pr"
 */
addToQueue(void* packet, int type, queue q, int pr);

int
/*
 * Pop the next queued packet.
 * Places the packet buffer location in the *buffer pointer
 * Returns the size of the packet
 */
popFromQueue(void** buffer, queue* q);

void
/*
 * Deallocates a queue
 */
delQueue(queue* q);

metadata meta;
node self;


#endif

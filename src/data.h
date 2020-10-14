#ifndef DATA
#define DATA
#include <stdint.h>

#define UNSET 255

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
	//byte version;
}metadata;

// TOUCH THESE :)
enum packet_types{
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

typedef struct{
	byte is_master;
	// ...
}node;

// An element on the inbound queue
typedef struct{
	time_t arrival_time;
	// ...	
}inbound_el;

// An element on the outbound queue
typedef struct{
	time_t departure_time;
	// ...	
}outbound_el;

// A queue
typedef struct{
	void* elements;
	int size;
}queue;




queue outbound_q;
queue inbound_q;
metadata meta;
node self;


#endif

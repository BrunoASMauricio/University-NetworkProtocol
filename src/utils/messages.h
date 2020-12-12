#ifndef PROTOCOL_BUILD_H
#define PROTOCOL_BUILD_H

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
	float PBE;				// link quality (perhaps not SNR)
} in_message;

/*
 * Generate a new message structure from
 * buffer
 */
void
newInMessage(in_message* msg, int size, void* buffer, timespec res);

/*
 * Clean an in_message structure
 * frees msg->buffer
 */
void
clearInMessage(in_message* msg);

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

#endif

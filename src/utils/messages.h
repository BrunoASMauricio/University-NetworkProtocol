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

#endif

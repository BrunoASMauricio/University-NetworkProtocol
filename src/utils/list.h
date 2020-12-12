#ifndef LIST
#define LIST

#include "../utils/macros.h"

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

short*
getIPFromList(IPList* IPL, int position);

void
/*
 * Insert an IP into an IP List
 * Only does so if it doesn't already exist
 * Allocates and copies IP
 * THREAD SAFE
 */
insertIPList(IPList* IPL, byte IP[2]);

void
/*
 * Remove an IP from an IP List
 * Frees IP
 * THREAD SAFE
 */
removeIPList(IPList* IPL, byte IP[2]);


// The following 7 functions are wrappers for the IP list functions
void
insertRegisteredSlave(byte IP[2]);

void
insertSubSlave(byte IP[2]);

bool
getSubSlave(byte IP[2]);

void
removeSubSlave(byte IP[2]);

void
insertSubSlave(byte IP[2]);

bool
getSubSlave(byte IP[2]);

void
removeSubSlave(byte IP[2]);



#endif

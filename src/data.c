#include "data.h"

queue*
newQueue()
{
	queue* Q = (queue*)malloc(sizeof(queue));
	Q->Size = 0;
	Q->First = NULL;
	Q->Last= NULL;
	
    if (pthread_mutex_init(&(Q->Lock), NULL) != 0) 
    {
        fatalErr("mutex init failed for outbound lock\n");
    }
	return Q;
}

/*
 * The reason for using linked lists instead of arrays
 * Is to allow for prioritized packets to "jump ahead" easily
 * If the memory allocation proves to be too slow or non-deterministic,
 * a pool of pre-allocated queue_el can be implemented
 */
void
addToQueue(void* Packet, int Size, queue* Q, int Pr)
{
	queue_el* NewEl = (queue_el*)malloc(sizeof(queue_el));
	queue_el *LastHigher;
	queue_el *ToSwap;

	NewEl->Packet = Packet;
	NewEl->PacketSize = Size;
	NewEl->Pr = Pr;
	NewEl->NextEl = NULL;

	pthread_mutex_lock(&(Q->Lock));
	ToSwap = Q->First;
	LastHigher = Q->First;

	while(ToSwap != NULL && ToSwap->Pr >= Pr) 
    {
		LastHigher = ToSwap;
		ToSwap = (queue_el*)ToSwap->NextEl;
	}

	if(Q->First == NULL) 
    {
        Q->First = NewEl;
		Q->Last = NewEl;
	} 
    else 
    {
		if(ToSwap == LastHigher) 
        {
			NewEl->NextEl= LastHigher;
			Q->First = NewEl;
		} 
        else 
        {
			if(ToSwap == NULL) 
            {
				Q->Last = NewEl;
			}
			NewEl->NextEl = LastHigher->NextEl;
			LastHigher->NextEl = NewEl;
		}
	}
	Q->Size += 1;
	pthread_mutex_unlock(&(Q->Lock));
}


void*
popFromQueue(int* Size, queue* Q)
{	
	queue_el* Popped;
	void* Buf;
	
	if (Q->Size == 0) 
    {
		return 0;
	}

	pthread_mutex_lock(&(Q->Lock));
	Popped = Q->First;
	Q->First = (queue_el*)Q->First->NextEl;
	Q->Size -= 1;
	pthread_mutex_unlock(&(Q->Lock));

	Buf = Popped->Packet;
	*Size = Popped->PacketSize;

	free(Popped);

	return Buf;
}

void
delQueue(queue* Q)
{
	pthread_mutex_destroy(&(Q->Lock));
	free(Q);
}

in_message* newMessage(int size, void* buffer, timespec res)
{
	in_message* ret = (in_message*)malloc(sizeof(in_message));

	ret->size = size;
	ret->buf = (void*)malloc(size);
	memcpy(ret->buf, buffer, size);
	ret->received_time = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;

	return ret;
}

void delMessage(in_message* Message)
{
	if(Message == NULL)
	{
		return;
	}
	free(Message->buf);
	free(Message);
}



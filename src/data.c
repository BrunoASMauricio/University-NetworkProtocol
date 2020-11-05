#include "data.h"

#define SETBIT(bit,byte) byte |= (1 << bit)
#define CLEARBIT(bit,byte) byte &= ~(1 << bit)

queue*
newQueue()
{
	queue* Q = (queue*)malloc(sizeof(queue));
	Q->Size = 0;
	Q->First = NULL;
	Q->Last= NULL;
	
    if (pthread_mutex_init(&(Q->Lock), NULL) != 0)
    {
        fatalErr("mutex init failed for queue lock\n");
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

List* newList()
{
	List* L = (List*)malloc(sizeof(List));
	L->First= NULL;
	L->Last= NULL;
	L->Size = 0;
	return L;
}

void delList(List* L)
{
	while(L->Size)
	{
		removeFromList(L, 0);
	}
	free(L);
}

void insertInList(List* L, void* buffer, int position)
{
	List_el* Helper;
	List_el* New_el;

	New_el = (List_el*)malloc(sizeof(List_el));
	New_el->Buff = buffer;
	New_el->Next= NULL;

	if(L->Size <= 0)
	{
		L->First = New_el;
		L->Last = New_el;
		L->Size += 1;
		return;
	}

	if(position >= L->Size)
	{
		L->Last->Next = New_el;
		L->Last = New_el;
	}
	else if(position <= 0)
	{
		New_el->Next = L->First;
		L->First = New_el;
	}
	else
	{
		Helper = (List_el*)L->First;

		for(int i = 0; i < position; i++)
		{
			Helper = (List_el*)Helper->Next;
		}

		New_el->Next = Helper->Next;
		Helper->Next = New_el;
	}
	L->Size += 1;
}

void printList(List* L)
{
	List_el* Helper = L->First;
	printf("List size: %d\n", L->Size);

	while(Helper != NULL)
	{
		printf("%s\n", Helper->Buff);
		Helper = (List_el*)Helper->Next;
	}
}

void* removeFromList(List* L, int position)
{
	void* buffer;
	List_el* Helper;
	List_el* ToFree;

	if(L->Size == 0 || position >= L->Size)
	{
		return NULL;
	}

	if(position == 0)
	{
		Helper = (List_el*)L->First->Next;
		buffer = L->First->Buff;
		free(L->First);
		L->First = Helper;
	}
	else
	{
		Helper = L->First;

		for(int i = 0; i < position-1; i++)
		{
			Helper = (List_el*)Helper->Next;
		}
		
		ToFree = (List_el*)Helper->Next;
		buffer = ToFree->Buff;
		Helper->Next = ToFree->Next;
		free(ToFree);
	}

	L->Size -= 1;

	return buffer;
}


IPList* newIPList()
{
	IPList* IPL = (IPList*)malloc(sizeof(IPList));
	IPL->L = newList();

	if (pthread_mutex_init(&(IPL->Lock), NULL) != 0)
    {
        fatalErr("mutex init failed for new IP list lock\n");
    }

	return IPL;
}

void delIPList(IPList* IPL)
{
	pthread_mutex_destroy(&(IPL->Lock));
	delList(IPL->L);
	free(IPL);
}


short*
getIPFromList(IPList* IPL, int position)
{
	List_el* Helper;
	
	if(position < 0 || position > IPL->L->Size)
	{
		return NULL;
	}

	pthread_mutex_lock(&(IPL->Lock));
	Helper = IPL->L->First;

	for(int i = 0; i < position; i++)
	{
		Helper = (List_el*)Helper->Next;
	}

	pthread_mutex_unlock(&(IPL->Lock));
	return (short*)Helper->Buff;
}

bool
getIPFromList(IPList* IPL, byte IP[2])
{
	List_el* Helper;
	pthread_mutex_lock(&(IPL->Lock));
	Helper = IPL->L->First;

	while(Helper != NULL)
	{
		if(((byte*)(Helper->Buff))[0] == IP[0] &&
		   ((byte*)(Helper->Buff))[1] == IP[1])
		{
			pthread_mutex_unlock(&(IPL->Lock));
			return true;
		}
		Helper = (List_el*)Helper->Next;
	}

	pthread_mutex_unlock(&(IPL->Lock));
	return false;
}

void
insertIPList(IPList* IPL, byte IP[2])
{
	List_el* Helper;
	pthread_mutex_lock(&(IPL->Lock));
	Helper = IPL->L->First;

	while(Helper != NULL)
	{
		if(((byte*)(Helper->Buff))[0] == IP[0] &&
		   ((byte*)(Helper->Buff))[1] == IP[1])
		{
			// Already exists
			pthread_mutex_unlock(&(IPL->Lock));
			return;
		}
		Helper = (List_el*)Helper->Next;
	}

	insertInList(IPL->L, IP, -1);
	pthread_mutex_unlock(&(IPL->Lock));
}

void
removeIPList(IPList* IPL, byte IP[2])
{
	List_el* Helper;
	pthread_mutex_lock(&(IPL->Lock));
	Helper = IPL->L->First;

	for(int i = 0; Helper != NULL; i++)
	{
		if(((byte*)(Helper->Buff))[0] == IP[0] &&
		   ((byte*)(Helper->Buff))[1] == IP[1])
		{
			// Not the most efficient way, but simple
			removeFromList(IPL->L, i);
			pthread_mutex_unlock(&(IPL->Lock));
			return;
		}
		Helper = (List_el*)Helper->Next;
	}

	pthread_mutex_unlock(&(IPL->Lock));
}

in_message* newInMessage(int size, void* buffer, timespec res)
{
	in_message* ret = (in_message*)malloc(sizeof(in_message));

	ret->size = size;
	ret->buf = (void*)malloc(size);
	memcpy(ret->buf, buffer, size);
	ret->received_time = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;

	return ret;
}

void delInMessage(in_message* Message)
{
	if(Message == NULL)
	{
		return;
	}

	free(Message->buf);
	free(Message);
}


out_message* newOutMessage(int size, void* buffer)
{
	out_message* ret = (out_message*)malloc(sizeof(out_message));

	ret->size = size;
	ret->buf = (void*)malloc(size);

	return ret;
}

void delOutMessage(out_message* Message)
{
	if(Message == NULL)
	{
		return;
	}
	
	free(Message->buf);
	free(Message);
}



#include "list.h"

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
short* getIPFromList(IPList* IPL, int position)
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

bool getIPFromList(IPList* IPL, byte IP[2])
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

void insertIPList(IPList* IPL, byte in_IP[2])
{
	List_el* Helper;
	byte* IP = (byte*)malloc(sizeof(byte)*2);
	memcpy(IP, in_IP, 2);
	pthread_mutex_lock(&(IPL->Lock));
	Helper = IPL->L->First;

	while(Helper != NULL)
	{
		if(((byte*)(Helper->Buff))[0] == IP[0] &&
		   ((byte*)(Helper->Buff))[1] == IP[1])
		{
			// Already exists
			pthread_mutex_unlock(&(IPL->Lock));
			free(IP);
			return;
		}
		Helper = (List_el*)Helper->Next;
	}

	insertInList(IPL->L, IP, -1);
	pthread_mutex_unlock(&(IPL->Lock));
}

void removeIPList(IPList* IPL, byte IP[2])
{
	List_el* Helper;
	byte* prevIP;
	pthread_mutex_lock(&(IPL->Lock));
	Helper = IPL->L->First;

	for(int i = 0; Helper != NULL; i++)
	{
		if(((byte*)(Helper->Buff))[0] == IP[0] &&
		   ((byte*)(Helper->Buff))[1] == IP[1])
		{
			// Not the most efficient way, but simple
			pthread_mutex_unlock(&(IPL->Lock));
			free(getIPFromList(IPL, i));
			removeFromList(IPL->L, i);
			return;
		}
		Helper = (List_el*)Helper->Next;
	}

	pthread_mutex_unlock(&(IPL->Lock));
}

void
insertSubSlave(byte IP[2])
{
	insertIPList(Self.SubSlaves, IP);
}

bool
getSubSlave(byte IP[2])
{
	return getIPFromList(Self.SubSlaves, IP);
}

void
removeSubSlave(byte IP[2])
{
	removeIPList(Self.SubSlaves, IP);
}

void
insertOutsideSlave(byte IP[2])
{
	insertIPList(Self.OutsideSlaves, IP);
}

bool
getOutsideSlave(byte IP[2])
{
	return getIPFromList(Self.OutsideSlaves, IP);
}

void
removeOutsideSlave(byte IP[2])
{
	removeIPList(Self.OutsideSlaves, IP);
}


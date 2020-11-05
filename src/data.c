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
    memcpy(ret->buf, buffer, size);

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

/*
 * PBID-IP pairs table functions
 */

pbid_ip_table* pbidInitializeTable()
{
	pbid_ip_table* table_head = (pbid_ip_table*)malloc(sizeof(pbid_ip_table));
	if(table_head == NULL)
	{
		fatalErr("Memory allocation failed.\n");
	}

	table_head->first_pair = NULL;
	if(pthread_mutex_init(&(table_head->Lock), NULL) != 0)
	{
			fatalErr("mutex init failed for outbound lock\n");
	}


  pbid_ip_pairs* table = (pbid_ip_pairs*)malloc(sizeof(pbid_ip_pairs));
  if(table == NULL)
  {
    fatalErr("Memory allocation failed.\n");
  }

  table->PresentIP[0] = {0x00};
	table->PresentIP[1] = {0x00};
  table->PresentPBID[0] = {0x00};
	table->PresentPBID[1] = {0x00};
  table->next_pair = NULL;
	table->IsLastPair = true;

	pthread_mutex_lock(&(table_head->Lock));
	table_head->first_pair = table;
	pthread_mutex_unlock(&(table_head->Lock));

  return table_head;
}

void pbidInsertPair(byte* IP_ofPair, byte* PBID_ofPair, pbid_ip_table* table_head)
{
	pbid_ip_pairs* table = table_head->first_pair;
  //case1 - first pair of the table

  //just inserts it on previously initialized and allocated data of struct
  if(table->PresentIP[0] == 0x00 && table->PresentPBID[0] == 0x00
      && table->PresentIP[1] == 0x00 && table->PresentPBID[1] == 0x00)
	{
		pthread_mutex_lock(&(table_head->Lock));
		memcpy(table->PresentIP, IP_ofPair, sizeof(table->PresentIP));
		memcpy(table->PresentPBID, PBID_ofPair, sizeof(table->PresentPBID));
		pthread_mutex_unlock(&(table_head->Lock));

    return;
  }


  //case2 - not the first pair added to the table

	while(1)
  {
      if(memcmp(table->PresentIP, IP_ofPair, sizeof(table->PresentIP)) == 0)
      {
					pthread_mutex_lock(&(table_head->Lock));
          memcpy(table->PresentPBID, PBID_ofPair, sizeof(table->PresentPBID));
					pthread_mutex_unlock(&(table_head->Lock));

          return;
      }

			if(table->IsLastPair)
			{
				break;
			}

			table = table->next_pair;
  }

  //no hits -> new entry
  pbid_ip_pairs* new_entry = (pbid_ip_pairs*) malloc(sizeof(pbid_ip_pairs));

  if(new_entry == NULL)
  {
      fatalErr("Memory allocation failed.\n");
  }

  memcpy(new_entry->PresentIP, IP_ofPair, sizeof(new_entry->PresentIP));
  memcpy(new_entry->PresentPBID, PBID_ofPair, sizeof(new_entry->PresentPBID));
  new_entry->next_pair = NULL;
	new_entry->IsLastPair = true;

	pthread_mutex_lock(&(table_head->Lock));
	table->IsLastPair = false;
  table->next_pair = new_entry;
	pthread_mutex_unlock(&(table_head->Lock));

  return;
}



int pbidSearchPair(byte* IP_ofPair, byte* PBID_ofPair, pbid_ip_table* table_head)
{
	pbid_ip_pairs* table = table_head->first_pair;
  //if table is empty
  if(table->PresentIP[0] == 0x00 && table->PresentPBID[0] == 0x00
		&& table->PresentIP[1] == 0x00 && table->PresentPBID[1] == 0x00)
  {
    return 0;
  }

  //if table not empty
	pthread_mutex_lock(&(table_head->Lock));
	while(memcmp(table->PresentPBID, PBID_ofPair, sizeof(table->PresentPBID)) != 0)
  {
    table = table->next_pair;
    if(table == NULL)
    {
      //no hits
			pthread_mutex_unlock(&(table_head->Lock));
      return 0;
    }
  }

	pthread_mutex_unlock(&(table_head->Lock));
  return 1;
}



void pbidPrintTable(pbid_ip_table* table_head)
{
	pbid_ip_pairs* table = table_head->first_pair;
  while(table != NULL)
  {
    printf("\tIP: 0x%2x%2x\tPBID: 0x%2x%2x\tLast element: %d\n",
						table->PresentIP[1], table->PresentIP[0],
						table->PresentPBID[1], table->PresentPBID[0], table->IsLastPair);
    table = table->next_pair;
  }
}



void pbidRemovePair(byte* IP_toRemove, pbid_ip_table* table_head)
{
	pbid_ip_pairs* temp = table_head->first_pair;
	pbid_ip_pairs* prev;
	bool FirstElement = true;

	pthread_mutex_lock(&(table_head->Lock));
	while(temp != NULL &&
		memcmp(temp->PresentIP, IP_toRemove, sizeof(temp->PresentIP)) != 0)
	{
		if(FirstElement)
		{
			FirstElement = false;
		}
		prev = temp;
		temp = temp->next_pair;
	}

	if(temp == NULL)
	{
		pthread_mutex_unlock(&(table_head->Lock));
		return;
	}

	if(FirstElement)
	{
		if(temp->next_pair == NULL)
		{
			temp->PresentIP[0] = {0x00};
			temp->PresentIP[1] = {0x00};
			temp->PresentPBID[0] = {0x00};
			temp->PresentPBID[1] = {0x00};
			temp->IsLastPair = true;

			table_head->first_pair = temp;
			pthread_mutex_unlock(&(table_head->Lock));
			return;
		}

		table_head->first_pair = temp->next_pair;
		free(temp);
		pthread_mutex_unlock(&(table_head->Lock));
		return;
	}

	prev->next_pair = temp->next_pair;
	if(temp->IsLastPair)
	{
		prev->IsLastPair = true;
	}
	free(temp);
	pthread_mutex_unlock(&(table_head->Lock));
	return;
}

out_message* 
buildNERMessage(byte* NextHopIP, byte* OutsiderIP)
{
    
    byte packet[5];
    //NOTE(GoncaloXavier): Version | Packet Type
    packet[0] = (PROTOCOL_VERSION<<4) + NER;
    packet[1] = NextHopIP[0];
    packet[2] = NextHopIP[1];
    packet[3] = OutsiderIP[0];
    packet[4] = OutsiderIP[1];
    
    out_message* NERMessage = newOutMessage(Packet_Sizes[NER], packet);
    
    return NERMessage;
}

out_message* 
buildNEPMessage(byte* SenderIP, byte* OutsiderIP)
{
    void* buff;
    
    byte packet[5];
    //NOTE(GoncaloXavier): Version | Packet Type
    packet[0] = (PROTOCOL_VERSION<<4) + NEP;
    packet[1] = SenderIP[0];
    packet[2] = SenderIP[1];
    packet[3] = OutsiderIP[0];
    packet[4] = OutsiderIP[1];
    
    out_message* NEPMessage = newOutMessage(Packet_Sizes[NEP], packet);
    
    return NEPMessage;
}

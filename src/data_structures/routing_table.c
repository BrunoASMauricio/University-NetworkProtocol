#include "routing_table.h"

table* routNewTable()
{
    table *tbl = (table*) malloc(sizeof(table));

    if(tbl == NULL)
    {
        printf("Could not create routing table\n");
        return NULL;
    }

    if (pthread_mutex_init(&(tbl->lock), NULL) != 0)
    {
        fatalErr("mutex init failed for table lock\n");
    }

    tbl->begin = NULL;
    tbl->size=0; //starts with size 0

    return tbl;
}
void routUpdateRollingLocalPBE(byte NeighIP[2], float newLocalPBE, unsigned int bytes_heard)
{
    table_entry *entry = routSearchByIp(Self.Table,NeighIP);
	if(entry == NULL)
	{
		return;
	}
	printf(" (SD) %u.%u old local pbe %f  new local pbe %f  old bytes heard %d new bytes heard %d\n",NeighIP[0], NeighIP[1], entry->LocalPBE, newLocalPBE, entry->bytes_heard, bytes_heard);
	entry->LocalPBE = (entry->LocalPBE*entry->bytes_heard+newLocalPBE*bytes_heard)/(entry->bytes_heard+bytes_heard);
	printf("%f\n", entry->LocalPBE);
	if(entry->bytes_heard + bytes_heard > ROLLING_MAX)
	{
		entry->bytes_heard = ROLLING_RESTART;
	}
	else
	{
		entry->bytes_heard = entry->bytes_heard + bytes_heard;
	}

}
table_entry* routNewEntry(byte NeighIP[2], unsigned short Distance, float LocalPBE, float RemotePBE, unsigned long int LastHeard)
{
    /* allocate memory for new entry*/
    table_entry *Entry = (table_entry*)malloc(sizeof(table_entry));

    if(Entry == NULL) 
    {
        printf("Error in allocating table_entry memory\n");
        return NULL;
    }

    /* init with values*/
    memcpy(Entry->Neigh_IP, NeighIP, sizeof(Entry->Neigh_IP));
    
    Entry->Distance=Distance;
    /*check this after everything*/
    Entry->LocalPBE = LocalPBE;
    Entry->RemotePBE = RemotePBE;
    Entry->LastHeard = LastHeard;
	Entry->bytes_heard= 0;

    Entry->next=NULL;

    return Entry;
}
void routUpdateLastHeard(table * tbl, byte IP[2])
{
    pthread_mutex_lock(&(tbl->lock));
	table_entry* entr = routSearchByIp(tbl, IP);
	if(entr)
	{
		timespec Res;
		unsigned long int Act;
		clock_gettime(CLOCK_REALTIME, &Res);
		Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
		//printf("Updating last heard of %u.%u from %lu to %lu: %lu\n", IP[0], IP[1], entr->LastHeard, Act, Act-entr->LastHeard);
		entr->LastHeard = Act;
		//routInsertOrUpdateEntry(tbl, IP, entr->Distance, entr->LocalPBE, entr->RemotePBE, Act);
	}
    pthread_mutex_unlock(&(tbl->lock));
}

unsigned long int nextHopTimeout(table_entry* entry)
{
	// Ti(A) = (1+P[B->A]) * Kr * T[PB];
	unsigned long int ret = (entry->RemotePBE+1)*1UL;
	ret *= ((unsigned long int)(ROUTE_LOSS_WAITING_FACTOR*RETRANSMISSION_DELAY_PB_MAX));
	return ret;
}

byte* getBestHop()
{
	table_entry* entry;
	timespec Res;
	unsigned long int Act;
	bool had_connection = false;
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
	entry = Self.Table->begin;
	printf("Printing\n");
	routPrintTableContent(Self.Table);
	byte IP[2];
	while(entry != NULL && entry->LastHeard + nextHopTimeout(entry) < Act){
		printf("Lost connection to %u.%u\n", entry->Neigh_IP[0], entry->Neigh_IP[1]);
		had_connection = true;
		IP[0] = entry->Neigh_IP[0];
		IP[1] = entry->Neigh_IP[1];
		routRemoveEntry(Self.Table, entry->Neigh_IP);
		entry = Self.Table->begin;
	}
	if(had_connection && entry == NULL){
		controlledShutdown();
        return NULL;
	}
    if( Self.Table->begin == NULL ||
        Self.Table->begin->Neigh_IP == NULL ||
        getDistance(Self.Table->begin) == UNREACHABLE
        )
    {
		// Lost connection to Master, proceed with controlled shutdown
        return NULL;
    }
    return Self.Table->begin->Neigh_IP;
}
table_entry* routInsertOrUpdateEntry(table * tbl, byte NeighIP[2], unsigned short Distance, float LocalPBE, float RemotePBE, unsigned long int LastHeard, unsigned int bytes_heard)
{
    if(tbl == NULL) 
    {
       printf("Tried to insert/update a non-existent routTable\n");
       return NULL;
    }
	//printf("Adding route to %u.%u dist=%d local_pbe=%f remote_pbe=%f last heard = %lu\n",NeighIP[0], NeighIP[1], Distance, LocalPBE, RemotePBE, LastHeard);
    
    table_entry *aux = NULL;
    table_entry *aux1 = NULL;
    table_entry *aux2 = NULL;
   
    pthread_mutex_lock(&(tbl->lock));

    /*if the table is empty and we just have to put there the entry*/
    if (tbl->size == 0)
    {
        tbl->begin=routNewEntry(NeighIP, Distance, LocalPBE, RemotePBE, LastHeard);

            if(tbl->begin == NULL)
            {
                pthread_mutex_unlock(&(tbl->lock));
                return NULL;
            }

        tbl->size++;
        pthread_mutex_unlock(&(tbl->lock));
        return tbl->begin;
    }

    /* starts by checking if there's already an entry with the specified IP*/
    table_entry *entry = routSearchByIp(tbl,NeighIP);
   
    if(entry == NULL)
    {
        aux=routNewEntry(NeighIP, Distance, LocalPBE, RemotePBE,LastHeard);

            if(aux == NULL) 
            {
                pthread_mutex_unlock(&(tbl->lock));
                return NULL;
            }
            
        tbl->size++;
    } 
    else //if there's already an entry
    {
        //do the updates
		//	abs(entry->LocalPBE - LocalPBE) > RECEIVED_QUALITY_THRASHING_LIMIT ||
		//	abs(entry->RemotePBE - RemotePBE) > RECEIVED_QUALITY_THRASHING_LIMIT ||
		float StoreLocalPBE;
		unsigned int _heard_bytes;
		printf(" %u.%u old local pbe %f  new local pbe %f  old bytes heard %d new bytes heard %d\n",NeighIP[0], NeighIP[1], entry->LocalPBE, LocalPBE, entry->bytes_heard, bytes_heard);
		StoreLocalPBE = (entry->LocalPBE*entry->bytes_heard+LocalPBE*bytes_heard)/(entry->bytes_heard+bytes_heard);
		printf("%f\n", StoreLocalPBE);
		if(entry->bytes_heard + bytes_heard > ROLLING_MAX)
		{
			_heard_bytes = ROLLING_RESTART;
		}
		else
		{
			_heard_bytes = entry->bytes_heard + bytes_heard;
		}
		byte * Store_IP =(byte*)malloc(sizeof(byte)*2);
		memcpy(Store_IP, entry->Neigh_IP, sizeof(entry->Neigh_IP));
		routRemoveEntry(tbl, entry->Neigh_IP);
		aux=routNewEntry(Store_IP, Distance, StoreLocalPBE, RemotePBE, LastHeard);
		aux->bytes_heard = _heard_bytes;
		tbl->size++;
		/*
		if(abs(entry->Distance - old_distance) > RECEIVED_DISTANCE_THRASHING_LIMIT)
		{
		}
		else{
			pthread_mutex_unlock(&(tbl->lock));
			return entry;
		}
		*/
    }
	
	//TODO: REFACTOR ROUT FUNCTIONS
	//FIRST test if you didn't just freed the tbl->begin! 
	//if you did, just place it on the beginning
	if(tbl->begin == NULL)
	{
		tbl->begin = (table_entry*)malloc(sizeof(table_entry));
		tbl->begin = aux;
		pthread_mutex_unlock(&(tbl->lock));
		return aux;
	}

    /*
    * we start by checking if there's only one entry on the table, which makes things easier
    */

    if(tbl->begin->next == NULL) //if it exists and there's only one entry in the table, we are talking about the same entry
    {  
        if (getDistance(tbl->begin) < getDistance(aux))
        {   
            tbl->begin->next=aux;
            tbl->begin->next->next=NULL;
            pthread_mutex_unlock(&(tbl->lock));
            return aux;
        }
        else
        {
            aux1= tbl->begin;
            tbl->begin = aux;
            tbl->begin->next=aux1;
            pthread_mutex_unlock(&(tbl->lock));
            return aux;
        }
    }

    if(tbl->begin->next != NULL) //if there's more than 2 entries and we have to insert a complety new one
    {
        aux1=tbl->begin;
        aux2=aux1;

        while( aux1 != NULL)
        {
            if(getDistance(aux) < getDistance(aux1)) break;

            aux2=aux1; //to store the previous
            aux1=aux1->next;
        }
       
        if(aux1==aux2) //special case when we have to insert on the head
        {
           tbl->begin=aux;
           aux->next=aux1;
           pthread_mutex_unlock(&(tbl->lock));
           return aux;
        }
        
        //general case
        aux->next=aux2->next;
        aux2->next=aux; 
        pthread_mutex_unlock(&(tbl->lock));
        return aux;
    }
    
    pthread_mutex_unlock(&(tbl->lock));
    return NULL;
}
int routPrintTableContent(table *tbl)
{
    pthread_mutex_lock(&(tbl->lock));
    table_entry *aux = tbl->begin;
    int n=0;

    while(aux != NULL)
    {
        printf( "Table size: %d Ip[0]: %d, Ip[1]: %d, Distance: %u Next: %p Last heard: %lu\n",tbl->size, aux->Neigh_IP[0], aux->Neigh_IP[1], getDistance(aux), aux->next, aux->LastHeard);
        aux=aux->next;
        n++;
    }
    if (n != tbl->size) 
    {
        pthread_mutex_unlock(&(tbl->lock));
        return 0;
    }
    else 
    {
        pthread_mutex_unlock(&(tbl->lock));
        return tbl->size;
    }
}
table_entry* routSearchByIp(table *tbl, byte neigh_IP[2])
{
    table_entry *Ptr = tbl->begin;

    while(Ptr != NULL)
        {
            if(memcmp(Ptr->Neigh_IP, neigh_IP, sizeof(Ptr->Neigh_IP)) == 0)
            {
                return Ptr;
            }

            Ptr = Ptr->next;
        }
    return NULL;
}
bool routRemoveEntry(table *tbl, byte neigh_IP[2])
{
    table_entry *Current = NULL;
    table_entry *Prev = NULL;
 
    if(tbl->begin==NULL) 
    {
        //printf("Empty routing table\n");
        pthread_mutex_unlock(&(tbl->lock));
        return false;
    } 

    if(memcmp(tbl->begin->Neigh_IP, neigh_IP, sizeof(tbl->begin->Neigh_IP)) == 0)
    {
        if(tbl->begin->next!= NULL)
        {
            Current = tbl->begin;
            tbl->begin = tbl->begin->next;
            free(Current);
            tbl->size--;
        
            return true;
        } 
        else 
        {
            free(tbl->begin);
            tbl->begin = NULL;
            tbl->size--;
            //printf("Routing table is now empty\n");
         
            return true;
        }
    }
    else if((memcmp(tbl->begin->Neigh_IP, neigh_IP, sizeof(tbl->begin->Neigh_IP)) != 0) && (tbl->begin->next == NULL)) 
    {
        printf("IP %d.%d not found in table\n", neigh_IP[0],neigh_IP[1]);

        return false;
    }

    Current = tbl->begin;
   
    while(Current->next != NULL && memcmp(Current-> Neigh_IP, neigh_IP, sizeof(tbl->begin->Neigh_IP)) != 0) 
    {
        Prev = Current;
        Current = Current->next;
    }        

    if((memcmp(Current-> Neigh_IP, neigh_IP, sizeof(tbl->begin->Neigh_IP)) == 0)) 
    {
        Prev->next = Prev->next->next;
        free(Current);  
        tbl->size--;
    } 
    else printf("IP %d.%d not found in table\n", neigh_IP[0],neigh_IP[1]);
     
    return false;
}

table_entry* routGetEntryByPos(table *tbl, int pos)
{
    if(tbl == NULL)
    { 
        printf("Tbl pointer is null\n");
        return NULL;
    }

    pthread_mutex_lock(&(tbl->lock));

    table_entry* current = tbl->begin;
    table_entry* store = NULL;
    int i =0;

    while(current != NULL && i !=pos)
    {
        store=current;
        current=current->next;
        i++;
    }
    
    pthread_mutex_unlock(&(tbl->lock));
    return store;
}

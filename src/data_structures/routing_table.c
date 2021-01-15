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
byte* getBestHop()
{
    if( Self.Table->begin == NULL ||
        Self.Table->begin->Neigh_IP == NULL ||
        getDistance(Self.Table->begin) == UNREACHABLE
        )
    {
        return NULL;
    }
    return Self.Table->begin->Neigh_IP;
}
table_entry* routInsertOrUpdateEntry(table * tbl, byte NeighIP[2], unsigned short Distance, float LocalPBE, float RemotePBE, unsigned long int LastHeard)
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
		if(	abs(entry->LocalPBE - LocalPBE) > RECEIVED_QUALITY_THRASHING_LIMIT ||
			abs(entry->RemotePBE - RemotePBE) > RECEIVED_QUALITY_THRASHING_LIMIT ||
		   	abs(entry->Distance - Distance)> RECEIVED_DISTANCE_THRASHING_LIMIT)
		{
			byte * Store_IP =(byte*)malloc(sizeof(byte)*2);
			memcpy(Store_IP, entry->Neigh_IP, sizeof(entry->Neigh_IP));
			float StoreLocalPBE= entry->LocalPBE;
			float StoreRemotePBE= entry->RemotePBE;
			routRemoveEntry(tbl, entry->Neigh_IP);
			aux=routNewEntry(Store_IP, Distance, LocalPBE, RemotePBE, LastHeard);
			tbl->size++;
		}
		else{
			pthread_mutex_unlock(&(tbl->lock));
			return entry;
		}
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
        if (getDistance(tbl->begin) <= getDistance(aux))
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
            if(getDistance(aux) <= getDistance(aux1)) break;

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

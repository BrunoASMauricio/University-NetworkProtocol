#include "routing_table.h"


table* newTable()
{

     table *tbl = (table*) malloc(sizeof(table));

    if(tbl == NULL)
    {
        printf("Could not create routing table\n");
        return NULL;
    }
    if (pthread_mutex_init(&(tbl->lock), NULL) != 0)
        {
        //fatalErr("mutex init failed for table lock\n");
        }


    tbl->begin = NULL;
    tbl->size=0; //starts with size 0

    return tbl;
}

table_entry* newEntry(byte NeighIP[2], double Quality, double AvgSnr, double EffectiveQuality)
{

    /* allocate memory for new entry*/
    table_entry *Entry = (table_entry*)malloc(sizeof(table_entry));
    if (Entry==NULL) return NULL;

    /* init with values*/
    memcpy(Entry->Neigh_IP,NeighIP, sizeof(Entry->Neigh_IP));
    
    Entry->Quality=Quality;

    /*check this after everything*/
    Entry->AvgSnr=AvgSnr;
    Entry->EffectiveQuality= EffectiveQuality;

    Entry->next=NULL;

    return Entry;

}

table_entry* insertOrUpdateEntry(table * tbl, byte NeighIP[2], double Quality, double AvgSnr, double EffectiveQuality)
{
    if(tbl == NULL) return NULL;

    table_entry *aux = NULL;
    table_entry *aux1 = NULL;
    table_entry *aux2 = NULL;
   
    pthread_mutex_lock(&(tbl->lock));

    /*if the table is empty and we just have to put there the entry*/
    if (tbl->size == 0)
    {
        tbl->begin=newEntry(NeighIP, Quality, AvgSnr, EffectiveQuality);
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
    table_entry *entry = searchByIp(tbl,NeighIP);
   
    if(entry == NULL)
    {
        aux=newEntry(NeighIP, Quality, AvgSnr, EffectiveQuality);
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
        byte * Store_IP =(byte*)malloc(sizeof(byte)*2);
        memcpy(Store_IP, entry->Neigh_IP, sizeof(entry->Neigh_IP));
        double StoreAvg= entry->AvgSnr;
        double StoreEff= entry->EffectiveQuality;
        
        removeEntry(tbl, entry->Neigh_IP);

        aux=newEntry(Store_IP, Quality, StoreAvg, StoreEff);
        tbl->size++;
    }
    /*
    * we start by checking if there's only one entry on the table, which makes things easier
    */

    if(tbl->begin->next == NULL) //if it exists and there's only one entry in the table, we are talking about the same entry
    {  
        if (tbl->begin->Quality > aux->Quality)
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

        while( aux1 != NULL){

            if(aux->Quality > aux1->Quality) break;

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

int printTableContent(table *tbl)
{

    pthread_mutex_lock(&(tbl->lock));
    table_entry *Aux = tbl->begin;
    int n=0;

    while(Aux != NULL){

        printf( "Table size: %d Ip[0]: %d, Ip[1]: %d, Quality: %lf Next: %p\n",tbl->size, Aux->Neigh_IP[0], Aux->Neigh_IP[1], Aux->Quality, Aux->next);
        Aux=Aux->next;
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
table_entry* searchByIp(table *tbl, byte neigh_IP[2])
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

bool removeEntry(table *tbl, byte neigh_IP[2])
{

    table_entry *Current = NULL;
    table_entry *Prev = NULL;
 
    if(tbl->begin==NULL) 
    {
        printf("Empty routing table\n");
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
            printf("Routing table is now empty\n");
         
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
    else printf("IP %d.%d not found in table\n", neigh_IP[0],neigh_IP[1] );
     
    return false;
}



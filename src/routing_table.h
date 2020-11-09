#ifndef TABLE_H

typedef uint8_t byte;

/**
 * entry struct represents table entry, it contains the neighbours nodes' IP as index,
 * effective quality, LocalSNR, Quality
 * single-linked list
*/

typedef struct table_entry
{
    byte Neigh_IP[2];
    short Distance;
    short LocalSNR; //this probably needs to be updated after math stuff
    short  RemoteSNR; //this probably needs to be updated after math stuff
    unsigned long int LastHeard;
    struct table_entry *next;
} table_entry;

/**
 * table structure, contains a pointer to it's beginning, the size and it's thread safe (we mainly only lock and unlock on the insert and update function
 * since that is the function that calls the others)
 */
 
 typedef struct table
 {
    table_entry *begin;
    int size;
	pthread_mutex_t lock;
 } table;

/********************************/

/**
 * creates an empty table
 * return : pointer to newly created table
 *          if it fails to be created, returns null
 */
table* routNewTable();
/**
 * creates new entry in the table (not yet in the actual table)
 * return: pointer to newly created entry
 *         if it fails to be created, returns null 
 */
table_entry* routNewEntry(byte NeighIP[2], double Distance, double LocalSNR, double RemoteSNR, unsigned long int LastHeard);

/**
 * inserts new entry on the routing table or updates an entry if the received IP is already there
 * returns newly inserted or updated entry
 * returns NULL if anything goes wrong
 */

table_entry* routInsertOrUpdateEntry(table * tbl, byte NeighIP[2], short Distance, short LocalSNR, short RemoteSNR, unsigned long int LastHeard);
/**
 * prints routing table's content
 * return: number of entries if there's no problems
 *         0 if tbl->size doesn't match the number of loop's iteration
 */
int routPrintTableContent(table *tbl);

/** 
 * removes an entry of the routing table;
 * receives as parameter the associated table and the IP associated with the entry;
 * if the inserted IP doesn't exists, returns null;
*/
bool routRemoveEntry(table *tbl, byte neigh_IP[2]);

/** 
 * searches an entry of the table
 * receives as parameter the IP associated with the entry
 * returns a pointer to the table entry associated with the IP.
 */
table_entry* routSearchByIp(table *tbl, byte neigh_IP[2]);

/**
 * returns entry by pos
 * returns null if anything goes wrong
 */
table_entry* routGetEntryByPos(table *tbl, int pos);
 

#define TABLE_H
#endif

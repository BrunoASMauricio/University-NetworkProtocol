#ifndef DATA_STRUCTURES_PBID
#define DATA_STRUCTURES_PBID

typedef struct pbid_ip_pairs{
    byte PresentIP[2];
    byte PresentPBID[2];
    struct pbid_ip_pairs* next_pair;
		bool IsLastPair;
} pbid_ip_pairs;

typedef struct{
		pbid_ip_pairs* first_pair;
		pthread_mutex_t Lock;
} pbid_ip_table;

pbid_ip_table* pbidInitializeTable();
/*
 * intializes the PBID-IP pairs table as a linked list
 * values of first PBID-IP pair are set to 0
 * returns a pointer to head of table
 *
 * head of table holds pointer to first PBID-IP pair
 * a PBID-IP pair is constituted by a PBID entry and an IP entry
 */



void pbidInsertPair(byte* IP_ofPair, byte* PBID_ofPair, pbid_ip_table* table_head);
/*
 * stores a PBID-IP pair ("IP_ofPair" and "PBID_ofPair") on the table pointed by "table_head"
 *
 * to do so, it first checks if the IP entry of pair to be added already exists:
 *   if yes - switchs only its corresponding PBID entry (discards previous PBID);
 *   if no - adds an entirely new pair
 */

pbid_ip_pairs* pbidSearchPair(byte* IP_ofPair, byte* PBID_ofPair, pbid_ip_table* table_head);
/*
 * should always be performed BEFORE pbid_storePair as stated on the protocol
 * returns 1 if the pair of "IP_ofPair" and "PBID_ofPair" is found on the table pointed by "table_head",
 * 				0 otherwise
 *
 * the search is made by checking only the PBID entrys as this is an unique
 * identifier, meaning that in case of a PBID match, there's forcibly a match
 * between the pairs' corresponding IPs
 */



void pbidPrintTable(pbid_ip_table* table_head);
/*
 * prints all elements of the table pointed by "table_head"
 */



void pbidRemovePair(byte* IP_toRemove, pbid_ip_table* table_head);
/*
 * removes pair with "IP_toRemove" from the table pointed by "table_head"
 */

/*
 * Builds NEP Type Message using newOutMessage()
 */ 
#endif

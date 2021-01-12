#include "pbid_table.h"

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
			temp->PresentIP[0] = 0x00;
			temp->PresentIP[1] = 0x00;
			temp->PresentPBID[0] = 0x00;
			temp->PresentPBID[1] = 0x00;
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

void emptyTable(pbid_ip_table** table_head)
{
	pbid_ip_pairs* table;
	table = (*table_head)->first_pair;
	while(table != NULL && !(table->PresentIP[0] == 0x00 && table->PresentIP[1] == 0x00))
	{
		pbidRemovePair(table->PresentIP, *table_head);
		table = (*table_head)->first_pair;
	}
	*table_head = pbidInitializeTable();
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
pbid_ip_pairs* pbidSearchIP(byte* IP_ofPair, byte* PBID_ofPair, pbid_ip_table* table_head)
{
	pbid_ip_pairs* table = table_head->first_pair;
  //if table is empty
  if(table->PresentIP[0] == 0x00 && table->PresentPBID[0] == 0x00
		&& table->PresentIP[1] == 0x00 && table->PresentPBID[1] == 0x00)
  {
    return NULL;
  }

  //if table not empty
	pthread_mutex_lock(&(table_head->Lock));
	while(memcmp(table->PresentIP, IP_ofPair, sizeof(table->PresentIP)) != 0)
  {
    table = table->next_pair;
    if(table == NULL)
    {
      //no hits
			pthread_mutex_unlock(&(table_head->Lock));
      return NULL;
    }
  }

	pthread_mutex_unlock(&(table_head->Lock));
  return table;
}

pbid_ip_pairs* pbidSearchPair(byte* IP_ofPair, byte* PBID_ofPair, pbid_ip_table* table_head)
{
	pbid_ip_pairs* table = table_head->first_pair;
  //if table is empty
  if(table->PresentIP[0] == 0x00 && table->PresentPBID[0] == 0x00
		&& table->PresentIP[1] == 0x00 && table->PresentPBID[1] == 0x00)
  {
    return NULL;
  }

  //if table not empty
	pthread_mutex_lock(&(table_head->Lock));
	while(memcmp(table->PresentPBID, PBID_ofPair, sizeof(table->PresentPBID)) != 0 || memcmp(table->PresentIP, IP_ofPair, sizeof(table->PresentIP)) != 0)
  {
    table = table->next_pair;
    if(table == NULL)
    {
      //no hits
			pthread_mutex_unlock(&(table_head->Lock));
      return NULL;
    }
  }

	pthread_mutex_unlock(&(table_head->Lock));
  return table;
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

  table->PresentIP[0] = 0x00;
	table->PresentIP[1] = 0x00;
  table->PresentPBID[0] = 0x00;
	table->PresentPBID[1] = 0x00;
  table->next_pair = NULL;
	table->IsLastPair = true;

	pthread_mutex_lock(&(table_head->Lock));
	table_head->first_pair = table;
	pthread_mutex_unlock(&(table_head->Lock));

  return table_head;
}










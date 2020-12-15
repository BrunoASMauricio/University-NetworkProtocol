#include "equations.h"


unsigned short getDistance(table_entry* t_en)
{
	// Full routing conversation hasn't taken place yet
	if(t_en->LocalPBE == 1 || t_en->RemotePBE == 1 || t_en->Distance == UNREACHABLE)
	{
		return 0;
	}
	return (unsigned short)((1-t_en->LocalPBE)*10+(1-t_en->RemotePBE)*45+(1-(float)t_en->Distance/(float)UNREACHABLE)*45);
    //return ((unsigned short)((float)((31-t_en->LocalPBE)/t_en->LocalPBE+100)))+1U+t_en->Distance;
}

/*
 * Returns the current distance to the Master
 */
//unsigned short myDistance (float Quality, int NumberOfSubSlaves, int MaxThroughput)
unsigned short myDistance()
{
    table_entry* FirstEntry = routGetEntryByPos(Self.Table, 1);
	if(!FirstEntry)
	{
		printf("TB SIZE: %d %p DISTANCE: 0\n", Self.Table->size, FirstEntry);
		return UNREACHABLE;
	}
	printf("TB SIZE: %d %p DISTANCE: %u\n", Self.Table->size, FirstEntry, getDistance(FirstEntry));
	return getDistance(FirstEntry);
}

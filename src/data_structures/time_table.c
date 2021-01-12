#include "time_table.h"


void clearBitmapValue(short* IP, void* bitmap, int size, void* IPs)
{
	int place = -1;
	byte* local_byte;

	for(int i = 0; i < size; i++)
	{
		if(((short*)IPs)[i] == IP[0])
		{
			place = i;
			break;
		}
	}

	if(place == -1)
	{
		printf("Could not find IP %u.%u in bitmap\n", ((byte*)IP)[0], ((byte*)IP)[1]);
		dumpBin((char*)IPs, size*2, "IPs: ");
		return;
	}
	local_byte = (byte*)bitmap + (place/8);
	place = place - 8 * (place/8);
	dumpBin((char*)bitmap, 1, " size %d place: %d local_byte: 0x%02hhx\n", size, place, local_byte[0]);
	local_byte[0] = CLEARBIT(7-place, local_byte[0]);
}


bool getBitmapValue(short* IP, void* bitmap, int size, void* IPs)
{
	int place = -1;
	byte* local_byte;

	for(int i = 0; i < size; i++)
	{
		if(((short*)IPs)[i] == IP[0])
		{
			place = i;
			break;
		}
	}

	if(place == -1)
	{
		printf("Could not find IP %u.%u in bitmap", ((byte*)IP)[0], ((byte*)IP)[1]);
		dumpBin((char*)IPs, size*2, "IPs: ");
		return false;
	}
	local_byte = (byte*)bitmap + (place/8);
	place = place - 8 * (place/8);
	return (0x80 >> place) & local_byte[0];
}

void checkNewTimeTable(unsigned long int act)
{
	if(pthread_mutex_trylock(&(Self.NewTimeTable->Lock)))
	{
		return;
	}
	if(Self.NewTimeTable->sync && Self.NewTimeTable->sync < act)
	{
		printf("Setting new timetable %lu %lu\n");
		Self.TimeTable->local_slot =	Self.NewTimeTable->local_slot;
		Self.TimeTable->sync = 			Self.NewTimeTable->sync;
		Self.TimeTable->timeslot_size =	Self.NewTimeTable->timeslot_size;
		Self.TimeTable->table_size =	Self.NewTimeTable->table_size;
		Self.NewTimeTable->sync = 0;
	}
	pthread_mutex_unlock(&(Self.NewTimeTable->Lock));
}

timetable* newTimeTable()
{
	timetable* tm = (timetable*)malloc(sizeof(timetable));

	if(pthread_mutex_init(&(tm->Lock), NULL) != 0)
    {
        fatalErr("mutex init failed for new IP list lock\n");
    }
	tm->sync = 0;
	return tm;
}


void delTimeTable(timetable* tm)
{
	pthread_mutex_destroy(&(tm->Lock));
	free(tm);
}

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
		printf("Could not find IP in bitmap");
		dumpBin((char*)IPs, size, "IPs: ");
	}
	dumpBin((char*)bitmap, 1, " size %d place: %d\n", size, place);
	local_byte = (byte*)bitmap + (place/8);
	place = place - 8 * (place/8);
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
		printf("Could not find IP in bitmap");
		dumpBin((char*)IPs, size, "IPs: ");
		return false;
	}
	local_byte = (byte*)bitmap + (place/8);
	place = place - 8 * (place/8);
	return (0x80 >> place) & local_byte[0];
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

#ifndef DATA_STRUCTURES_TIME_TABLE
#define DATA_STRUCTURES_TIME_TABLE

// All 64 bits to avoid operation mistakes
// Most of the operations would be 64 bits either way
typedef struct{
	pthread_mutex_t Lock;
	unsigned long int timeslot_size;	// timeslot size (1 byte padded) in ns
	unsigned long int table_size;		// timetable size (2 bytes padded) in ns
	unsigned long int  local_slot;		// local slot (0 to N(umber of nodes)) (1 byte padded)
	unsigned long int sync;				// sync time in ns
} timetable;

void clearBitmapValue(short* IP, void* bitmap, int size, void* IPs);


bool getBitmapValue(short* IP, void* bitmap, int size, void* IPs);

void checkNewTimeTable(unsigned long int act);

timetable* newTimeTable();


void delTimeTable(timetable* tm);


#endif

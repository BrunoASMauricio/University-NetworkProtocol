#include "debug.h"

void dumpBin(char* buf, int size, const char *fmt,...)
{
	va_list args;
	if(Meta.Quiet)
	{
		return;
	}

	va_start(args, fmt);
	pthread_mutex_lock(&(Self.OutputLock));
	fprintf(stdout, "[%c] [!]", getThreadChar());
	vfprintf(stdout, fmt, args);

	for(int i = 0; i < size; i++)
	{
		fprintf(stdout, "0x%02hhx ", buf[i]);
	}

	fprintf(stdout, "\n");
	pthread_mutex_unlock(&(Self.OutputLock));

	va_end(args);
}
void printMessage(void* buff, int size)
{
	printf("Message (%d bytes):\n");
	switch (((byte*)buff)[0] & 0x0f)
	{
		case SD:
			printf("\tSD");
			break;
		case PB:
			printf("\tPB");
			break;
		case PR:
			printf("\tPR");
			break;
		case PC:
			printf("\tPC");
			break;
		case TA:
			printf("\tTA");
			break;
		case TB:
			printf("\tTB");
			break;
		case NE:
			printf("\tNE");
			break;
		case NEP:
			printf("\tNEP");
			break;
		case NER:
			printf("\tNER");
			break;
		case NEA:
			printf("\tNEA");
			break;
	}
	dumpBin((char*)buff, size, "\n\tDump: ");
	printf("\n");
	fflush(stdout);
}

char getThreadChar()
{
	pthread_t se = pthread_self();
	if(se == Meta.WF_listener_t)
	{
		return 'R';
	}
	else if(se == Meta.WF_dispatcher_t)
	{
		return 'T';
	}
	else if(se == Meta.WS_listener_t)
	{
		return 'S';
	}
	else if(se == Meta.HW_dispatcher_t)
	{
		return 'H';
	}
	else if(se == Meta.Retransmission_t)
	{
		return 'E';
	}
	else if(se == Meta.Main_t)
	{
		return 'M';
	}
	else
	{
		return 'X';
	}
}
void
printfLog(const char *fmt, ...)
{
	va_list args;
	if(!Meta.Quiet)
	{
		int hours, minutes, seconds, day, month, year;

		// time_t is arithmetic time type
		time_t now;

		// Obtain current time
		// time() returns the current time of the system as a time_t value
		time(&now);
		struct tm *local = localtime(&now);
		hours = local->tm_hour;          // get hours since midnight (0-23)
		minutes = local->tm_min;         // get minutes passed after the hour (0-59)
		seconds = local->tm_sec;         // get seconds passed after minute (0-59)

		day = local->tm_mday;            // get day of month (1 to 31)
		month = local->tm_mon + 1;       // get month of year (0 to 11)
		year = local->tm_year + 1900;    // get year since 1900

		va_start(args, fmt);
		pthread_mutex_lock(&(Self.OutputLock));
		fprintf(stdout, "[%c] [!] [%02d:%02d:%02d]", getThreadChar(), hours, minutes, seconds);
		vfprintf(stdout, fmt, args);
		pthread_mutex_unlock(&(Self.OutputLock));
		va_end(args);
	}

	fflush(stdout);
}
void
fatalErr(const char *fmt, ...)
{
	FILE* t;
	va_list args;

	if((t = fopen("./fatal", "w")) != NULL) 
    {
		va_start(args, fmt);
		vfprintf(t, fmt, args);
		fflush(t);
		va_end(args);
	}
    else
    {
		fprintf(stderr, "Could not create \"fatal\" file\n");
	}

	va_start(args, fmt);
	fprintf(stderr, "[XX]");
    vfprintf(stderr, fmt, args);
	fflush(stderr);
    va_end(args);
	
	clean();
	exit(EXIT_FAILURE);
}

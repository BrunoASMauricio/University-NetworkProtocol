#include "debug.h"

void dumpBin(char* buf, int size, const char *fmt,...)
{
	va_list args;
	if(Meta.Quiet)
	{
		return;
	}

	va_start(args, fmt);
	fprintf(stdout, "[%c] [!]", getThreadChar());
	vfprintf(stdout, fmt, args);

	for(int i = 0; i < size; i++)
	{
		fprintf(stdout, "0x%02hhx ", buf[i]);
	}

	fprintf(stdout, "\n");

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
		fprintf(stdout, "[%c] [!]", getThreadChar());
		va_start(args, fmt);
		vfprintf(stdout, fmt, args);
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

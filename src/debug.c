#include "debug.h"

void dumpBin(char* buf, int size, const char *fmt,...)
{
	va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
	for(int i = 0; i < size; i++)
    {
		fprintf(stdout, "0x%02hhx ", buf[i]);
	}
	fprintf(stdout, "\n");

	va_end(args);
	if (Meta.Log)
    {
		va_start(args, fmt);
		vfprintf(Meta.Log, fmt, args);
		for(int i = 0; i < size; i++)
        {
			fprintf(Meta.Log, "0x%02hhx ", buf[i]);
		}
		fprintf(Meta.Log, "\n");
		va_end(args);
	}
}

void
printfLog(const char *fmt, ...)
{
	va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
	va_end(args);
	if (Meta.Log) {
		fprintf(Meta.Log, "[!] ");
		va_start(args, fmt);
		vfprintf(Meta.Log, fmt, args);
		va_end(args);
		//fflush(Meta.Log);
	}
	fflush(stdout);
}


void
printfErr(const char *fmt, ...)
{
	if (!Meta.Debug) return;
	va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
	if (Meta.Log) {
		fprintf(Meta.Log, "[X] ");
		va_start(args, fmt);
		vfprintf(Meta.Log, fmt, args);
		va_end(args);
		//fflush(Meta.Log);
	}
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

void 
testRoutingTable()
{

    table *Tbl;
    
    Tbl=newTable();

    printf("Entry with lowest distance must be on the top...\nPlease check if the table size corresponds with the announced one\n");

    printf("Adding a 1st entry...\n");
    
    byte IP[2]={2,3};
    short Qual = 8888;
    short Avg = 222;
    short Eff = 777;

    table_entry *Entry;
    Entry=insertOrUpdateEntry(Tbl,IP,Qual,Avg,Eff);
    if(Entry==NULL) printf("Failed to insert entry\n");

    printTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 2nd entry...\n");

    byte IP_[2]={4,4};
    short Qual1 = 88;
    short Avg1 = 222;
    short Eff1 = 777;
     
    table_entry *Entry1;
    Entry1=insertOrUpdateEntry(Tbl, IP_, Qual1, Avg1, Eff1);

    printTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 3rd entry...\n");

    byte IP_0[2]={7,8};
    short Qual2 = 1000;
    short Avg2 = 222;
    short Eff2 = 777;

    table_entry *Entry2;
    Entry2=insertOrUpdateEntry(Tbl, IP_0, Qual2, Avg2, Eff2);

    printTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 4th entry...\n");

    byte IP_1[2]={1,1};
    short Qual3 = 10000;
    short Avg3 = 222;
    short Eff3 = 777;

    table_entry *Entry3;
    Entry3=insertOrUpdateEntry(Tbl, IP_1, Qual3, Avg3, Eff3);  

    printTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 5th entry...\n");

    byte IP_2[2]={2,2};
    short Qual4 = 2000;
    short Avg4 = 222;
    short Eff4 = 777;
     
    table_entry *Entry4;
    Entry4=insertOrUpdateEntry(Tbl, IP_2, Qual4, Avg4, Eff4);  

    printTableContent(Tbl);
    printf("\n\n");

    printf("Updating entry with IP: 11 ...\n");

    short QualUpd = 1;
    insertOrUpdateEntry(Tbl, IP_1, QualUpd, Avg4, Eff4);  

    printTableContent(Tbl);
    printf("\n\n");

    printf("Updating entry with IP: 44 ...\n");

    short QualUpd_1 = 100;
    insertOrUpdateEntry(Tbl, IP_, QualUpd_1, Avg4, Eff4);  

    printTableContent(Tbl);
    printf("\n\n");

    printf("Updating entry with IP: 11 ...\n");
    short QualUpd_2 = 1500;
    insertOrUpdateEntry(Tbl, IP_1, QualUpd_2, Avg4, Eff4);  

    printTableContent(Tbl);
    printf("\n\n");

	printf("Getting first position... \n");
	table_entry* entry5;
	entry5= getEntryByPos(Tbl, 1);

	printf("1st entry distance : %hi, 1st entry NextHop_IP: %d%d\n", entry5->Distance, entry5->Neigh_IP[0],entry5->Neigh_IP[1] );

	printf("Getting second position... \n");
	entry5= getEntryByPos(Tbl, 2);
	printf("2nd entry distance : %hi, 2nd entry NextHop_IP: %d%d\n", entry5->Distance, entry5->Neigh_IP[0],entry5->Neigh_IP[1] );

}

void
testQueues()
{
	queue* aq;
	char buff1[24] = "bean good, how bout you";
	char buff2[19] = "hello how you bean";
	char buff3[18] = "very nice, thanks";
	char buff4[14] = "just a packet";
	int dummy;
	char* dummy2;

	aq	= newQueue();
	addToQueue((void*)buff4, strlen(buff4), aq, -2);
	addToQueue((void*)buff3, strlen(buff3), aq, -1);
	addToQueue((void*)buff1, strlen(buff1), aq, 0);
	addToQueue((void*)buff1, strlen(buff1), aq, 0);
	addToQueue((void*)buff2, strlen(buff2), aq, 10);
	addToQueue((void*)buff4, strlen(buff4), aq, -2);

	printf("Expected output: %s %s %s\n",  buff2, buff2, buff2);
	printf("%s\n%s\n%s\n%s\n%s\n%s\n\n",
					buff2, buff1, buff1,
					buff3, buff4, buff4);

	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);
	
	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);
	
	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);
	
	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);

	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);

	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n\n", dummy2);

	delQueue(aq);	
}

void
performMeasurements(){
	struct timespec res;
	struct timespec res2;

	long avg = 0;
	long total = 0;
	for(int i = 0; i < 100; i++)
	{
		clock_gettime(CLOCK_REALTIME, &res);
		clock_gettime(CLOCK_REALTIME, &res2);
		avg += ((res2.tv_sec - res.tv_sec) * (int64_t)1000000000UL) + (res2.tv_nsec - res.tv_nsec);
		total++;
		// sleep(1); <-- this changes the average, why
	}
	printf("clock_gettime(CLOCK_REAL_TIME) average sampling delay: %lld\n", avg/total);
}

void
testAll(){
	char a[6];
	a[0] = 0xaf;
	a[1] = 0x99;
	a[2] = 0x82;
	a[3] = 0xff;
	a[4] = 0x00;
	a[5] = 0x11;

	printf("Starting protocol test\n---------\n");

	printf("Testing printfLog. Expected output:\n");
	printf(">>1 teste 0.5\n");
	printf(">>%d teste %0.1f\n", 1, 0.5);

	printf("Testing dumpBin. Expected output:\n");
	printf(">>Hello 4: 0xaf 0x99 0x82 0xff 0x00 0x11\n");
	
	dumpBin(a, sizeof(a), ">>Hello %d: ", 4);

	printf("Testing queue data structures\n");

	testQueues();
	testRoutingTable();




	printf("Ending protocol test\n---------\n");
	printf("Starting protocol measurements\n---------\n");
	performMeasurements();
	printf("Ending protocol measurements\n---------\n");


}

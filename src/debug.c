#include "debug.h"
#include "data.h"
#include "routing_table.h"
#include <stdio.h>

void dumpBin(char* buf, int size, const char *fmt,...)
{
	va_list args;
	if(Meta.Quiet)
	{
		return;
	}

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
		va_start(args, fmt);
		vfprintf(stdout, fmt, args);
		va_end(args);
	}

	if(Meta.Log)
	{
		fprintf(Meta.Log, "[%c] [!]", getThreadChar());
		va_start(args, fmt);
		vfprintf(Meta.Log, fmt, args);
		va_end(args);
	}

	fflush(stdout);
}


void
printfErr(const char *fmt, ...)
{
	if (!Meta.Debug) return;
	va_list args;
	if(!Meta.Quiet)
	{
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}

	if (Meta.Log)
	{
		fprintf(Meta.Log, "[%c] [X]", getThreadChar());
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
	timespec Res;
	unsigned long int Act;

	
    Tbl=routNewTable();

    printf("Entry with lowest distance must be on the top...\nPlease check if the table size corresponds with the announced one\n");

    printf("Adding a 1st entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    
    
    byte IP[2]={2,3};
    short Qual = 8888;
    short Avg = 222;
    short Eff = 777;

    table_entry *Entry;
    Entry=routInsertOrUpdateEntry(Tbl,IP,Qual,Avg,Eff, Act);
    if(Entry==NULL) printf("Failed to insert entry\n");

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 2nd entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    byte IP_[2]={4,4};
    short Qual1 = 88;
    short Avg1 = 222;
    short Eff1 = 777;

     
    table_entry *Entry1;
    Entry1=routInsertOrUpdateEntry(Tbl, IP_, Qual1, Avg1, Eff1, Act);

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 3rd entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    byte IP_0[2]={7,8};
    short Qual2 = 1000;
    short Avg2 = 222;
    short Eff2 = 777;

    table_entry *Entry2;
    Entry2=routInsertOrUpdateEntry(Tbl, IP_0, Qual2, Avg2, Eff2, Act);

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 4th entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    byte IP_1[2]={1,1};
    short Qual3 = 10000;
    short Avg3 = 222;
    short Eff3 = 777;

    table_entry *Entry3;
    Entry3=routInsertOrUpdateEntry(Tbl, IP_1, Qual3, Avg3, Eff3, Act);  

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 5th entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    byte IP_2[2]={2,2};
    short Qual4 = 2000;
    short Avg4 = 222;
    short Eff4 = 777;

    table_entry *Entry4;
    Entry4=routInsertOrUpdateEntry(Tbl, IP_2, Qual4, Avg4, Eff4, Act);  

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Updating entry with IP: 11 and changed last heard...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    short QualUpd = 1;
    routInsertOrUpdateEntry(Tbl, IP_1, QualUpd, Avg4, Eff4, Act);  

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Updating entry with IP: 44 ...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    
    short QualUpd_1 = 100;
    routInsertOrUpdateEntry(Tbl, IP_, QualUpd_1, Avg4, Eff4,Act);  

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Updating entry with IP: 11 ...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    
    short QualUpd_2 = 1500;
    routInsertOrUpdateEntry(Tbl, IP_1, QualUpd_2, Avg4, Eff4, Act);  

    routPrintTableContent(Tbl);
    printf("\n\n");

	printf("Getting first position... \n");
	table_entry* entry5;
	entry5= routGetEntryByPos(Tbl, 1);

	printf("1st entry distance : %hi, 1st entry NextHop_IP: %d%d\n", entry5->Distance, entry5->Neigh_IP[0],entry5->Neigh_IP[1] );

	printf("Getting second position... \n");
	entry5= routGetEntryByPos(Tbl, 2);
	printf("2nd entry distance : %hi, 2nd entry NextHop_IP: %d%d\n", entry5->Distance, entry5->Neigh_IP[0],entry5->Neigh_IP[1] );

}

void testTimeTable()
{
	unsigned long int Timeslot_size;
	unsigned long int Table_size;
	unsigned long int Local_slots;
	unsigned long int Sync;

	unsigned long int Act;
	unsigned long int Slot;
	unsigned long int Vact;
	unsigned long int Next;

	unsigned long int TEST_TRANSMISSION_DELAY = 10000UL;	//10 us
	timespec Res;

	unsigned long int Startedtimeslot = 0;
	unsigned long int Endedtimeslot = 0;

	double Usedtimeslot = 0;

	byte Started = 0;
	byte Where = 0;
	unsigned int Total = 0;

	printf("Starting timetable measurements.\n");
	printf("1 s timetable with 0.1s timeslots.\n");
	printf("Local timeslot is number 3 (so the fourth)\n");

	Timeslot_size = (int64_t)100000UL; // 0.1 s
	Table_size = (int64_t)2000000UL; // 2 ms
	Local_slots = (int64_t)4; // 3rd
	
	printf("Size %d\n", sizeof(Local_slots));
	clock_gettime(CLOCK_REALTIME, &Res);
	Sync = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;// + (int64_t)1000000000UL;

	for(unsigned int i = 0; i < 100000000 /*INT_MAX-1*/; i++)
	{
		clock_gettime(CLOCK_REALTIME, &Res);
		Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

		Slot = Sync + Local_slots * Timeslot_size;
		Vact = Act - Slot;
		Next = Table_size * ((Vact/Table_size) + 1) + Slot;
		
		if(Act < Sync || Act < Slot)	// Timetable isn't valid or first timeslot hasn't elapsed
		{
			continue;
		}
		if(Started == 0)
		{
			// Started!
			Started = 1;
		}

		if (Vact < Table_size * (Vact/Table_size) + Timeslot_size - TEST_TRANSMISSION_DELAY)
		{
			if (Where == 0)
			{
			   Where = 1;
			   Total += 1;
			}
		   	if(!Startedtimeslot)
			{
				Startedtimeslot = Act;
		   	}
		   	Endedtimeslot = Act;
		}
		else
		{
			if(Startedtimeslot)
			{
				Usedtimeslot = (100 * ((double)(Endedtimeslot-Startedtimeslot))/((double)Timeslot_size) + Usedtimeslot * Total)/(Total + 1);
				Startedtimeslot = 0;
			}
			Where = 0;
			// Tried to make the thread sleep, but failed miserably (for now)
			//clock_gettime(CLOCK_REALTIME, &res);
			//act = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;
			//printf("Sleeping for %ld", (next-act)/100UL);
			//usleep((next-act)/1000UL);
			//printf("Next %lu\n", next);
		}
	}
	printf("Used timeslot: %lf%\n", Usedtimeslot);
	printf("Used total: %lf%\n", 100 * (Usedtimeslot / 100 * (double)Timeslot_size) / Table_size);
	printf("Expected/Ideal:\nUsed timeslot: 90%\nUsed total: 5%\n");
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

	printf("Testing queue data structures\n");

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
testLists()
{
	List* L = newList();
	printf("Testing lists\n");
	char buff0[10] = "EL0";
	char buff1[10] = "EL1";
	char buff2[10] = "EL2";
	insertInList(L, buff0, -1);
	insertInList(L, buff1, 1);
	insertInList(L, buff2, 5);
	removeFromList(L, 1);
	removeFromList(L, 1);
	removeFromList(L, 1); // These two should not work because now there is
	removeFromList(L, 1); // only 1 element (position 0)
	printf("Got:\n");
	printList(L);
	printf("Expected:\nList size: 1\nEL0\n");
	delList(L);
}

void
testIPLists()
{
	IPList* IPL = newIPList();
	byte IP0[2] = {1,2};
	byte IP1[2] = {3,4};
	byte IP2[2] = {5,6};

	printf("Testing IP Lists\n");
	printf("Got:\n");
	printf("%d\n", getIPFromList(IPL, IP0));
	insertIPList(IPL, IP0);
	insertIPList(IPL, IP0);
	printf("%d\n", getIPFromList(IPL, IP0));
	insertIPList(IPL, IP1);
	removeIPList(IPL, IP0);
	printf("%d ", IPL->L->Size);
	printf("%d ", getIPFromList(IPL, IP1));
	printf("%d\n", getIPFromList(IPL, IP0));
	printf("Expected:\n0\n1\n1 1 0\n");
	delIPList(IPL);
}

void
test_PBID_IP_table()
{
	byte* etcI = (byte*)malloc(2*sizeof(byte));
	byte* etcP = (byte*)malloc(2*sizeof(byte));

	pbid_ip_table* lmao = pbidInitializeTable();

	int choice;

	while(1)
	{
		printf("\nPress\n0 to insert pair\t1 to search pair\t2 to remove pair\t9 to end test\n");
		scanf("%d", &choice);

		if(choice == 0)
		{
			printf("\ninsert IP to add (2 bytes in hex - something like 19a5)\n");
			scanf("%2hhx%2hhx", &etcI[1], &etcI[0]);
			printf("insert PBID to add (2 bytes in hex)\n");
			scanf("%2hhx%2hhx", &etcP[1], &etcP[0]);


			pbidInsertPair(etcI, etcP, lmao);

			printf("Printing table\n");
			pbidPrintTable(lmao);
		}
		else if(choice == 1)
		{
			printf("\ninsert IP of pair to search for (2 bytes in hex - something like 19a5)\n");
			scanf("%2hhx%2hhx", &etcI[1], &etcI[0]);
			printf("insert PBID of pair to search for (2 bytes in hex)\n");
			scanf("%2hhx%2hhx", &etcP[1], &etcP[0]);

			if(pbidSearchPair(etcI, etcP, lmao))
				printf("FOUND!\n");
			else
				printf("not found :(\n");
		}
		else if(choice == 2)
		{
			printf("\ninsert IP of pair to be removed (2 bytes in hex - something like 19a5)\n");
			scanf("%2hhx%2hhx", &etcI[1], &etcI[0]);
			pbidRemovePair(etcI, lmao);

			printf("Printing updated table\n");
			pbidPrintTable(lmao);
		}
		else if(choice == 9)
			break;
	}
}

void
testPacketSize(){
	/*
	printf("Testing PacketSize\n");

	char SD_Test[999] = {0xff, 0x02, 0x99, ...};
	char TB_Test[999] = {0xff, 0x02, 0x99, ...};
	if(PacketSize(SD_Test) != 999)
	{
		printfErr("Packet Size returned wrong value. Expected %d, got %d\n", PacketSize(SD_Test), 999);
	}
	if(PacketSize(TB_Test) != 999)
	{
		printfErr("Packet Size returned wrong value. Expected %d, got %d\n", PacketSize(SD_Test), 999);
	}
	*/
}

void
performMeasurements()
{
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
mockTB_RX(void* buff)
{
	bool send_TA = false;
	bool retransmit_TB = false;
	byte* local_byte;
	short* IPHolder;
	int ip_amm;
	byte slot;
	// New PBID? Accept new timeslot
	
	pthread_mutex_lock(&(Self.TimeTable->Lock));
	if(true)
	{
		Self.TimeTable->local_slot = -1;
		ip_amm = ((short*)(((byte*)buff+16)))[0];
		Self.TimeTable->table_size= ip_amm;
		for(int i = 0; i < ip_amm; i++)
		{
			if(((short*)(((byte*)buff+18)))[i] == ((short*)Self.IP)[0])
			{
				Self.TimeTable->local_slot = i;
				slot = i;
				printf("Our slot: %d\n",i);
				break;
			}
		}
		if(Self.TimeTable->local_slot == -1)
		{
			dumpBin((char*)buff, getPacketSize(buff), "Did not receive timeslot from TB\n");
			// SET STATE TO OUTSIDE NETWORK
			return;
		}
		Self.TimeTable->timeslot_size = (((byte*)buff+15))[0];
	}

	local_byte = ((byte*)buff)+18+ip_amm*2 + (slot/8);
	slot = slot - 8 * (slot/8);
	send_TA = (0x80 >> slot) & local_byte[0];

	printf("Should I send a TA? %d\n", send_TA);
	for(int i = 0; i < Self.SubSlaves->L->Size; i++)
	{
		IPHolder = getIPFromList(Self.SubSlaves, i);
		if(i == 2)
		{
			((byte*)IPHolder)[0] = 9;
		}
		local_byte = (byte*)getBitmapValue(IPHolder, (byte*)buff+18+ip_amm*2, ip_amm, (byte*)buff+18);
		retransmit_TB |= (bool)local_byte;
		printf("Should I retransmit TB because of %d %d? %d\n", ((byte*)IPHolder)[0], ((byte*)IPHolder)[1], local_byte);
	}
	if(send_TA)
	{
		// TA_TX()
	}
	if(retransmit_TB)
	{
		// TB_TX()
	}
	pthread_mutex_unlock(&(Self.TimeTable->Lock));
	return;
}

void
testTB()
{
	byte SubSlave0[2] = {1,2};
	byte SubSlave1[2] = {3,4};
	byte SubSlave2[2] = {5,6};
	byte OurIP[2] = {7,8};
	byte prev_ip[2];
	timespec res;
	void* buff;
	
	prev_ip[0] = Self.IP[0];
	prev_ip[1] = Self.IP[1];

	Self.IP[0] = OurIP[0];
	Self.IP[1] = OurIP[1];

	Self.SubSlaves = newIPList();

	insertSubSlave(SubSlave0);
	insertSubSlave(SubSlave1);
	insertSubSlave(SubSlave2);
	
	buff = generateTB();
	printf("TB Size\nGot: %d\nExpected: 27\n", getPacketSize(buff));fflush(stdout);
	dumpBin((char*)buff, getPacketSize(buff), "Dumping TB:\n");

	printf(" IN %lu\n", ((unsigned long int*)((byte*)buff+5))[0]);
	Self.TimeTable = newTimeTable();
	mockTB_RX(buff);
	delTimeTable(Self.TimeTable);
	Self.TimeTable = NULL;

	removeSubSlave(SubSlave0);
	removeSubSlave(SubSlave1);
	removeSubSlave(SubSlave2);
	Self.IP[0] = prev_ip[0];
	Self.IP[1] = prev_ip[1];

	delIPList(Self.SubSlaves);
	Self.SubSlaves = NULL;
}

void
testNE_RX()
{
    printf("\nTesting NE_RX;\n");
    // Setting some dummy Self values to test 
    Self.SubSlaves = newIPList();
    Self.OutsideSlaves = newIPList();
    Self.Table = routNewTable();
    Self.InboundQueue = newQueue();
    Self.OutboundQueue = newQueue();
    
    Self.IP[0] = 0x03;
    Self.IP[1] = 0x04;
    Self.Status = Inside;
    byte dummyPacket[5] = {(PROTOCOL_VERSION<<4) + NE, 0x01, 0x02, 0x03, 0x04};
    
    timespec Res;
    clock_gettime(CLOCK_REALTIME, &Res);
    
    in_message* NEreceived = newInMessage(5, dummyPacket, Res);
    dumpBin((char*)(NEreceived->buf), NEreceived->size, ">>Sent this to NE_RX: %X");
    NE_RX(NEreceived);

    printf("\nExpected to have added IP 0x01 0x02 as OutsideSlave:\n");
    byte OutsideSlaveIP[2] = {0x01, 0x02};
    printf("Is OutsideSlave present? %s",
            getIPFromList(Self.OutsideSlaves, OutsideSlaveIP) ? "YES\n": "NO\n");
    
    printf("Expected to have updated LastHeard on routTable\n");
    table_entry* Outsider = routSearchByIp(Self.Table, &dummyPacket[1]);
    printf("Current LastHeard: %lu\n", Outsider->LastHeard);
    
    if(Self.IsMaster)
    {
        printf("\nMASTER CASE:\n");
        printf("Expected to have generated TB\n");
        printf("Expected to have sent NEP\n");
    }
    else
    {
        printf("\nSLAVE CASE:\n");
        printf("Expected to have sent NEP and NER\n");
       
        printf("Expected to have started Retransmission of NER\n");
        printf("Checking if Self.Rt.Retransmitables is set on NER:\n");
        printf("Self.Rt.Retransmitables on NER:%s\n", 
                CHECKBIT(rNER, Self.Rt.Retransmitables) ? "YES\n": "NO\n");
    }
    
    printf("\nFinished NE_RX;\n");
}

void
testBuildNER()
{
    byte NextHopIP[2] = {0x01, 0x02};
    byte OutsiderIP[2] = {0x03, 0x04};
    out_message* NERpacket = buildNERMessage(NextHopIP ,OutsiderIP);
    printf("\nTesting buildNERMessage.\n Expected output:\n");
    printf(">>NERpacket: 5 0x%X 0x01 0x02 0x03 0x04\n", 
                                (PROTOCOL_VERSION<<4)+NER, NextHopIP, OutsiderIP );
    printf("Actual output:\n");
    dumpBin((char*)(NERpacket->buf), NERpacket->size, ">>NERpacket: %X ");
    printf("\nFinished testing BuildNER;\n");
}

void
testBuildNEA()
{
    byte OutsiderIP[2] = {0x01, 0x02};
    pbid PBID = 0;
    out_message* NEApacket = buildNEAMessage(OutsiderIP, PBID);
    printf("\nTesting buildNEAMessage.\n Expected output:\n");
    printf(">>NEApacket: 5 0x%X 0x01 0x02 %X %X\n", 
                                (PROTOCOL_VERSION<<4)+NEA, 
                                (PBID >> 8) & 0xff),
                                (PBID & 0xff);
    printf("Actual output:\n");
    dumpBin((char*)(NEApacket->buf), NEApacket->size, ">>NEApacket: %X ");
    printf("\nFinished testing BuildNEA;\n");
}

void
testNEA_RX()
{
	printf("\nTesting NEA_RX;\n");
    // Setting some dummy Self values to test 
    Self.OutsideSlaves = newIPList();
    Self.OutsidePending = newIPList();
    Self.OutboundQueue = newQueue();
    SETBIT(rNER, Self.Rt.Retransmitables);
    
    byte OutsiderIP[2];
    OutsiderIP[0] = 0x03;
    OutsiderIP[1] = 0x04;
    byte dummyPacket[5] = {(PROTOCOL_VERSION<<4) + NEA, 
                           0x03, 0x04, 0x01, 0x02};

    timespec Res;
    clock_gettime(CLOCK_REALTIME, &Res);
    
    printf("\nTesting direct connection to OutsideSlave 1st\n");
    insertOutsideSlave(OutsiderIP);
    in_message* NEAreceived = newInMessage(5, dummyPacket, Res);
    NEA_RX(NEAreceived);
    
    printf("Expected to have stoped startRetransmission on NER\n");
    printf("Self.Rt.Retransmitables on NER:%s\n", 
            CHECKBIT(rNER, Self.Rt.Retransmitables) ? "YES\n": "NO\n");
    
    removeOutsideSlave(OutsiderIP);
    printf("\nNow testing indirect connection to OutsideSlave\n");
    
    insertIPList(Self.OutsidePending, OutsiderIP);
    printf("getIPFromList got: %d\n", getIPFromList(Self.OutsidePending, OutsiderIP));
    NEAreceived = newInMessage(5, dummyPacket, Res);
    NEA_RX(NEAreceived);
    printf("Expected to have removed OutsiderIP from OutsiderPending\n");
    printf("getIPFromList got: %d\n", getIPFromList(Self.OutsidePending, OutsiderIP));
    
    delIPList(Self.SubSlaves);
    Self.SubSlaves = NULL;
    delIPList(Self.OutsideSlaves);
    Self.OutsideSlaves = NULL;

    delQueue(Self.OutboundQueue);
    return;
}

void
testNER_RX()
{
    printf("\nTesting NER_RX;\n");
    // Setting some dummy Self values to test 
	Self.SubSlaves = newIPList();
    Self.OutsideSlaves = newIPList();
    Self.OutsidePending = newIPList();
    Self.Table = routNewTable();
    Self.InboundQueue = newQueue();
    Self.OutboundQueue = newQueue();

    Self.IP[0] = 0x03;
    Self.IP[1] = 0x04;
    byte dummyPacket[5] = {(PROTOCOL_VERSION<<4) + NER, 
                           0x03, 0x04, 0x01, 0x02};
    routInsertOrUpdateEntry(Self.Table, &dummyPacket[3], 1, 1, 1, 1);
    
    timespec Res;
    clock_gettime(CLOCK_REALTIME, &Res);
    
    in_message* NERreceived = newInMessage(5, dummyPacket, Res);
    dumpBin((char*)(NERreceived->buf), NERreceived->size, ">>Sent this to NEP_RX: %X");
    NER_RX(NERreceived);

    printf("\nExpected to have added IP 0x01 0x02 as SubSlave:\n");
    byte SubSlaveIP[2] = {0x01, 0x02};
    printf("Is SubSlave present? %s",
            getIPFromList(Self.SubSlaves, SubSlaveIP) ? "YES\n": "NO\n");
    
    printf("Expected to have updated LastHeard on routTable\n");
    table_entry* Outsider = routSearchByIp(Self.Table, &dummyPacket[3]);
    printf("Current LastHeard: %lu\n", Outsider->LastHeard);
    
    if(Self.IsMaster)
    {
        printf("MASTER CASE:\n");
        printf("Expected to have generated TB\n");
        printf("Expected to have sent NEA\n");
    }
    else
    {
        printf("SLAVE CASE:\n");
        printf("Expected to have started startRetransmission on NER\n");
        printf("Checking if Self.Rt.Retransmitables is set on NER:\n");
        printf("Self.Rt.Retransmitables on NER:%s\n", 
                CHECKBIT(rNER, Self.Rt.Retransmitables) ? "YES\n": "NO\n");
    }
    
    printf("\nFinished NER_RX;\n");
}

void
testBuildNE()
{
    byte SourceIP[2] = {0x01, 0x02};
    byte DestIP[2] = {0x03, 0x04};
    out_message* NEpacket = buildNEMessage(SourceIP ,DestIP);
    printf("\nTesting buildNEMessage;\n Expected output:\n");
    printf(">>NEpacket: 5 0x%X 0x01 0x02 0x03 0x04\n", 
                                (PROTOCOL_VERSION<<4)+NE);
    printf("Actual output:\n");
    dumpBin((char*)(NEpacket->buf), NEpacket->size, ">>NEpacket: %X ");
    printf("\nFinished testing BuildNE;\n");
}

void
testBuildNEP()
{
    byte SourceIP[2] = {0x01, 0x02};
    byte DestIP[2] = {0x03, 0x04};
    out_message* NEPpacket = buildNEPMessage(SourceIP ,DestIP);
    printf("\nTesting buildNEPMessage;\n Expected output:\n");
    printf(">>NEPpacket: 5 0x%X 0x01 0x02 0x03 0x04\n", 
                                (PROTOCOL_VERSION<<4)+NEP);
    printf("Actual output:\n");
    dumpBin((char*)(NEPpacket->buf), NEPpacket->size, ">>NEPpacket: %X ");
    printf("\nFinished testing BuildNEP;\n");
}
    
void
testNEP_RX()
{
    printf("\nTesting NEP_RX;\n");
    // Setting some dummy Self values to test 
    Self.IP[0] = 0x03;
    Self.IP[1] = 0x04;
    Self.Status = Outside;
	SETBIT(rNE,Self.Rt.Retransmitables);
    byte dummyPacket[5] = {(PROTOCOL_VERSION<<4) + NEP, 0x01, 0x02, 0x03, 0x04};
    
    timespec Res;
    clock_gettime(CLOCK_REALTIME, &Res);
    
    in_message* NEPreceived = newInMessage(5, dummyPacket, Res);
    dumpBin((char*)(NEPreceived->buf), NEPreceived->size, ">>Sent this to NEP_RX: %X");
    NEP_RX(NEPreceived);
    
    printf("Expected to have STOPPED Retransmission on NE\n");
    printf("Checking if Self.Rt.Retransmitables is set on NE:\n");
    printf("Self.Rt.Retransmitables on NE:%s\n", 
            CHECKBIT(rNE, Self.Rt.Retransmitables) ? "YES\n": "NO\n");
    
    printf("Self.Status after NEP_RX\n"
            "Expected Waiting -> %d\n", Waiting);
    printf("Self.Status = %d\n", Self.Status);
    
    printf("\nFinished NEP_RX;\n");
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

	testQueues();
	
	testPacketSize();

	testRoutingTable();

    testNE_RX();
    testBuildNE();
	
    testBuildNEP();
    testNEP_RX();

    testBuildNER();
    testNER_RX();
    
	testTimeTable();

	testLists();

	testIPLists();

    testBuildNEA();
    testNEA_RX();
    
    testLists();

	testTB();

	printf("Iterative testing of PBID-IP pair table\n");
	test_PBID_IP_table();


	printf("Ending protocol test\n---------\n");
    /*
	printf("Starting protocol measurements\n---------\n");
	performMeasurements();
	printf("Ending protocol measurements\n---------\n");
    */
}

#include "time_table.h"

void
testBuildTA()
{
    byte Originator_IP[2] = {3, 4};
    byte PBID[2] = {5, 10};
    out_message* TAPacket = buildTAMessage(Originator_IP,PBID);

    printf("Actual output:\n");
    dumpBin((char*)(TAPacket->buf), TAPacket->size, ">>TApacket: %X ");

   
    printf("\nFinished testing BuildTA;\n");
}

void
testTA_RX()
{
	printf("\nTesting TA_RX;\n");

    // Setting some dummy Self values to test 
	byte prev_ip[2];
	byte OurIP[2] = {7,8};
	byte PBID[2];
	byte Originator_IP[2];
	byte SubSlave0[2] = {1,2};
	byte SubSlave1[2] = {3,4};
	byte SubSlave2[2] = {5,6};
	byte SubSlave3[2];
	Originator_IP[0] = 0x05;
	Originator_IP[1] = 0x06;

	prev_ip[0] = Self.IP[0];
	prev_ip[1] = Self.IP[1];


	Self.SubSlaves = newIPList();

	insertSubSlave(SubSlave0);
	insertSubSlave(SubSlave1);
	insertSubSlave(SubSlave2);
	
    OurIP[0] = 0x03;
    OurIP[1] = 0x04;
	Self.IP[0] = OurIP[0];
	Self.IP[1] = OurIP[1];
	PBID[0] = 0x07;
	PBID[1] = 0x08;
	void * TBmessage;
	TBmessage = generateTB();
	
    Self.IsMaster = true;
    byte dummyPacket[7] = {(PROTOCOL_VERSION<<4) + TA, Self.IP[0], Self.IP[1],  0x05, 0x06, PBID[0], PBID[1]};
    
    timespec Res;
    clock_gettime(CLOCK_REALTIME, &Res);
    
    in_message TAreceived;
	newInMessage(&TAreceived, 7, dummyPacket, Res);
    dumpBin((char*)(TAreceived.buf), TAreceived.size, ">>Sent this to TA_RX: %X");
   
	if(Self.IsMaster)
	{  // If Master, sets the corresponding Originator IPs' bit to 0 in the bitmap of the next TB retransmission

		// pthread_mutex_lock(&(Self.Rt.Lock));
		
        if(TBmessage == NULL)
			return;
		
		int ip_amm;
		ip_amm = Self.SubSlaves->L->Size;

		for(int i = 0; i < ip_amm; i++)
		{	
	
		  if(getIPFromList(Self.SubSlaves, i)[0] == ((short*)Originator_IP)[0])
		  {
				((byte*)TBmessage)[18+ip_amm*2+i] = 0;
                printf("bitmap of ip %u %u is equal to %d \n", Originator_IP[0], Originator_IP[1], ((byte*)TBmessage)[18+ip_amm*2+i]);
		  }
        
		 

		//pthread_mutex_unlock(&(Self.Rt.Lock));

		}
	}
	else
	{
			printf("Sending TA again...");
			// 	TA_TX(Originator_IP, PBID); 

		
	}
    
	Self.IP[0] = prev_ip[0];
	Self.IP[1] = prev_ip[1];
    delIPList(Self.SubSlaves);
	Self.SubSlaves = NULL;
	printf("\nFinished TA_RX;\n");
	free(TBmessage);
	clearInMessage(&TAreceived);
	return;

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

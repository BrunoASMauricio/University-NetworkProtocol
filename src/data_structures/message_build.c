#include "message_build.h"

void sendNetStats(netstat_type type)
{
	short* IPHolder;
	byte* payload;
	int payload_size;
	switch(type)
	{
		case STAT_TIMETABLE:
			pthread_mutex_lock(&(Self.SubSlaves->Lock));
			payload_size = 16+3+(Self.SubSlaves->L->Size+1)*2;
			payload = (byte*)malloc(sizeof(byte)*(payload_size));	// 16byte header, 3 bytes for timetable info, 2 bytes per IP (Account for self)
			for(int i = 0; i < 17; i++)
			{
				payload[i] = 0;
			}
			payload[16] = Self.TimeTable->timeslot_size;
			((short*)(&payload[17]))[0] = Self.TimeTable->table_size;
			payload[19] = Self.IP[0];
			payload[20] = Self.IP[1];
			for(int i = 0; i < Self.SubSlaves->L->Size; i++)
			{
				pthread_mutex_unlock(&(Self.SubSlaves->Lock));
				IPHolder = getIPFromList(Self.SubSlaves, i);
				pthread_mutex_lock(&(Self.SubSlaves->Lock));
				((short*)(&(payload[21+i*2])))[0] = *IPHolder;
				printf("TB IP %u.%u\n", ((byte*)IPHolder)[0], ((byte*)IPHolder)[1]);
			}
			pthread_mutex_unlock(&(Self.SubSlaves->Lock));
			dumpBin((char*)payload, payload_size, "TB information message\n");
			addToQueue((void*)payload, payload_size, Self.InternalQueue, 1);

			break;
			/*
		case STAT_ROUTING:
			payload = (byte*)malloc(sizeof(byte)*Self.Table->size);
			break;
			*/
	}

}

void* createPB()
{
	byte* PBPacket = (byte*)malloc(sizeof(byte)*7);
	
    if(PBPacket == NULL)
    {
        fatalErr("Couldn't assign memory to PB Packet \n");
    }

    table_entry* FirstEntry =routGetEntryByPos(Self.Table, 1);
    PBPacket[0]=(PROTOCOL_VERSION<<4)+PB;
    PBPacket[1]=Self.IP[0];
    PBPacket[2]=Self.IP[1];
    PBPacket[3]= (getNewPBID()>> 8) &0xff;
    PBPacket[4]= Self.PBID &0xff ;

	unsigned short Distance = myDistance();
	PBPacket[5]= (Distance >> 8) &0xff;
	PBPacket[6]= Distance &0xff;
	return PBPacket;
}
void* generateTB()
{
	timespec res;
	void* buff;
	byte rest;
	byte* IP;
	int ip_amm;
	short* IPHolder;
	int size;

	pthread_mutex_lock(&(Self.SubSlaves->Lock));
	printf("Building TB\n");
	ip_amm = Self.SubSlaves->L->Size + 1;	// Account for self
	size = ip_amm*(2*8+1);

	if(8*(size/8) != size)
	{
		size = size/8 + 1;
	}
	else
	{
		size /= 8;
	}
	size += Packet_Sizes[TB];

	buff = (void*)malloc(size);
	printf("Allocating %d bytes for TB\n", size);
	((byte*)buff)[0] = (0xf0 & (PROTOCOL_VERSION<<4)) | TB;
	((byte*)buff)[1] = Self.IP[0];
	((byte*)buff)[2] = Self.IP[1];
	((byte*)buff)[3] = 0;//Self.TB_PBID[0];
	((byte*)buff)[4] = 0;//Self.TB_PBID[1];
	((short*)Self.TB_PBID)[0] += 1;
	clock_gettime(CLOCK_REALTIME, &res);
	((unsigned long int*)((byte*)buff+5))[0] = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;
	printf(" OUT %lu\n", ((unsigned long int*)((byte*)buff+5))[0]);
	((short*)(((byte*)buff+13)))[0] = DEFAULT_VALIDITY_DELAY;
	(((byte*)buff+15))[0] = DEFAULT_TIMESLOT_SIZE;
	((short*)(((byte*)buff+16)))[0] = ip_amm;
	printf("Sub-Slave IP Ammount %d\n", ip_amm);

	((short*)(((byte*)buff+18)))[0] = ((short*)Self.IP)[0];
	for(int i = 1; i < ip_amm; i++)
	{
		pthread_mutex_unlock(&(Self.SubSlaves->Lock));
		IPHolder = getIPFromList(Self.SubSlaves, i-1);
		pthread_mutex_lock(&(Self.SubSlaves->Lock));
		((short*)(((byte*)buff+18)))[i] = IPHolder[0];
		printf("Adding IP: %d %d at %d\n", ((byte*)IPHolder)[0],  ((byte*)IPHolder)[1], 18+2*i);
	}

	printf("Adding bitmap at %d\n", 18+ip_amm*2);
	
	for(int i = 0; i < ip_amm/8; i++)
	{
		printf("Cool\n");
		((byte*)buff)[18+ip_amm*2+i] = 0xff;
	}
	rest = ip_amm - 8*(ip_amm/8);
	
	if(rest)
	{
		// No point in "cutting" the last bits, because the bitmap must
		// already cut them
		printf("Cool 2\n");
		((byte*)buff)[18+ip_amm*2+(ip_amm/8)] = (0xff<<(8-rest));
	}
	
	CLEARBIT(7, ((byte*)buff)[18+ip_amm*2+(ip_amm/8)]);

	dumpBin((char*)buff, getPacketSize(buff), "GENERATED TB: ");
	pthread_mutex_unlock(&(Self.SubSlaves->Lock));

	pthread_mutex_lock(&(Self.NewTimeTable->Lock));
	Self.NewTimeTable->local_slot = 0;
	unsigned long int validity_delay = (unsigned long int)(*((unsigned short*)(((byte*)buff+13))))*1E3;
	Self.NewTimeTable->sync = *((unsigned long int*)(((byte*)buff+5)));
	Self.NewTimeTable->timeslot_size = *(((byte*)buff+15))*1E6;
	ip_amm = *((short*)(((byte*)buff+16)));
	Self.NewTimeTable->table_size = ip_amm*Self.NewTimeTable->timeslot_size;
	Self.NewTimeTable->sync += validity_delay;
	pthread_mutex_unlock(&(Self.NewTimeTable->Lock));

	
	return buff;
}

out_message*
buildSDMessage(void* buff, int size, byte* IP)
{
	int TotalSize=0, NumSamples=0, Popped;

	byte packet[Packet_Sizes[SD]+MAX_PAYLOAD_SIZE];
   	//	= (byte*)malloc(sizeof(byte)*(Packet_Sizes[SD] + size));
	byte* NextHopIP = Self.Table->begin->Neigh_IP;
	//Assuming first position in table is itself; otherwise, search in table by self IP or something else

	if (NextHopIP == NULL)
	{
		printf("Next hop still undefined, dropping data message\n");
        return NULL;
	}

	packet[0] = (PROTOCOL_VERSION<<4) + SD;
	packet[1] = IP[0];
	packet[2] = IP[1];
	packet[3] = NextHopIP[0];
	packet[4] = NextHopIP[1];
	// To be replaced by Seq and TTL(set to a max of 8)
	packet[5] = (0<<4) + 8;
	packet[6] = size;
	memcpy(&packet[7], buff, size);

    out_message* SDmessage = newOutMessage(getPacketSize(packet), packet);
	return SDmessage;
}
out_message*
buildTAMessage(byte* Originator_IP, byte * PBID)
{
    int size = sizeof(byte)*7;
	byte* TAPacket = (byte*)malloc(size);

	if(TAPacket == NULL){
        fatalErr("Couldn't assign memory to TA Packet \n");
    }


    TAPacket[0]=(PROTOCOL_VERSION<<4)+TA;
    TAPacket[1]=Self.IP[0];
    TAPacket[2]=Self.IP[1];
    TAPacket[3]=Originator_IP[0];
    TAPacket[4]=Originator_IP[1];
    TAPacket[5]=PBID[0];
    TAPacket[6]=PBID[1];

    out_message* TAmessage = newOutMessage(getPacketSize(TAPacket), TAPacket);

    return TAmessage;
}
void* buildPRMessage(byte Originator_IP[2], byte PBID[2], float PBE)
{
	byte* PRPacket = (byte*)malloc(sizeof(byte)*13);

    PRPacket[0]=(PROTOCOL_VERSION<<4)+PR;
    PRPacket[1]=Self.IP[0];
    PRPacket[2]=Self.IP[1];
    PRPacket[3]=Originator_IP[0];
    PRPacket[4]=Originator_IP[1];
    PRPacket[5]=PBID[0];
    PRPacket[6]=PBID[1];
	unsigned short Distance = myDistance();
	PRPacket[7]=(Distance >> 8) &0xff;
	PRPacket[8]=Distance &0xff;
    *((float*)(&(PRPacket[9]))) = PBE;

	return PRPacket;
}

out_message*
buildNEPMessage(byte* SenderIP, byte* OutsiderIP)
{
    void* buff;

    byte packet[5];
    //Version | Packet Type
    packet[0] = (PROTOCOL_VERSION<<4) + NEP;
    packet[1] = SenderIP[0];
    packet[2] = SenderIP[1];
    packet[3] = OutsiderIP[0];
    packet[4] = OutsiderIP[1];

    out_message* NEPMessage = newOutMessage(Packet_Sizes[NEP], packet);

    return NEPMessage;
}

out_message*
buildNEAMessage(byte* OutsiderIP, pbid PBID)
{
    void* buff;

    byte packet[5];
    //Version | Packet Type
    packet[0] = (PROTOCOL_VERSION<<4) + NEA;
    packet[1] = OutsiderIP[0];
    packet[2] = OutsiderIP[1];
    packet[3] = (PBID >> 8) & 0xff;
    packet[4] = PBID & 0xff;

    out_message* NEAMessage = newOutMessage(Packet_Sizes[NEA], packet);

    return NEAMessage;
}

void*
buildNEMessage(byte* SenderIP, byte* ProxyIP)
{
    void* buff;
    
    byte* packet = (byte*)malloc(5);
    //Version | Packet Type
    packet[0] = (PROTOCOL_VERSION<<4) + NE;
    packet[1] = SenderIP[0];
    packet[2] = SenderIP[1];
    packet[3] = ProxyIP[0];
    packet[4] = ProxyIP[1];
    
	return packet;
}

void*
buildNERMessage(byte* NextHopIP, byte* OutsiderIP)
{
    byte* packet = (byte*)malloc(5);
    //Version | Packet Type
    packet[0] = (PROTOCOL_VERSION<<4) + NER;
    packet[1] = NextHopIP[0];
    packet[2] = NextHopIP[1];
    packet[3] = OutsiderIP[0];
    packet[4] = OutsiderIP[1];

	return packet;
}




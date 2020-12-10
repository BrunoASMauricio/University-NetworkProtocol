#include "message_build.h"


void* createPB()
{
	byte* PBPacket = (byte*)malloc(sizeof(byte)*7);
    short MasterDistance =0;
	
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

    if(Self.IsMaster == false)
    {
        PBPacket[5]= (FirstEntry->Distance >> 8) &0xff;
        PBPacket[6]= FirstEntry->Distance &0xff;
    }
    else
    {
        PBPacket[5]= (MasterDistance >> 8) &0xff;
        PBPacket[6]= MasterDistance &0xff;
    }
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
	((byte*)buff)[3] = Self.TB_PBID[0];
	((byte*)buff)[4] = Self.TB_PBID[1];
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
	
	return buff;
}

out_message*
buildTAMessage(byte* Originator_IP, byte * PBID)
{
    int size = sizeof(byte)*7;
	byte* TAPacket = (byte*)malloc(size);
	byte SELF_IP[2] = {7,8};

	if(TAPacket == NULL){
        fatalErr("Couldn't assign memory to TA Packet \n");
    }


    TAPacket[0]=(PROTOCOL_VERSION<<4)+TA;
    TAPacket[1]=SELF_IP[0];
    TAPacket[2]=SELF_IP[1];
    TAPacket[3]=Originator_IP[0];
    TAPacket[4]=Originator_IP[1];
    TAPacket[5]=PBID[0];
    TAPacket[6]=PBID[1];

    out_message* TAmessage = newOutMessage(getPacketSize(TAPacket), TAPacket);

    return TAmessage;
}
void* buildPRMessage(byte Originator_IP[2], byte PBID[2], byte SNR)
{
	byte* PRPacket = (byte*)malloc(sizeof(byte)*10);

    table_entry* FirstEntry = routGetEntryByPos(Self.Table, 1);
    PRPacket[0]=(PROTOCOL_VERSION<<4)+PR;
    PRPacket[1]=Self.IP[0];
    PRPacket[2]=Self.IP[1];
    PRPacket[3]=Originator_IP[0];
    PRPacket[4]=Originator_IP[1];
    PRPacket[5]=PBID[0];
    PRPacket[6]=PBID[1];
    PRPacket[7]=(FirstEntry->Distance >> 8) &0xff;
    PRPacket[8]=FirstEntry->Distance &0xff;
    PRPacket[9]=SNR;
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




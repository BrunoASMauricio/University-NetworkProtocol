#include "TX.h"


void*
WF_dispatcher(void* dummy)
{
	printf("WF Dispatcher on\n");
	while(1)
    {
        sleep(1);
    }
}

void
sendMessage(void* msg)
{

}

void*
HW_dispatcher(void*dummy)
{
	printf("HW Dispatcher on\n");
	while(1)
    {
        sleep(1);
    }
}


void SD_TX(int Sample_Ammount, void* Samples)
{
	return;
}

void PB_TX()
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
    PBPacket[3]= (Self.RoutingPBID >> 8) &0xff;
    PBPacket[4]= Self.RoutingPBID &0xff ;

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
 

	out_message* message = newOutMessage(getPacketSize(PBPacket), PBPacket);
    addToQueue(message, message->size, Self.OutboundQueue, 1);

	return;
}

void PR_TX(byte Originator_IP[2], byte PBID[2], byte SNR)
{
	byte* PRPacket = (byte*)malloc(sizeof(byte)*10);

    if(PRPacket == NULL)
    {
        fatalErr("Couldn't assign memory to PB Packet \n");
    }

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

	out_message* message = newOutMessage(getPacketSize(PRPacket), PRPacket);
	addToQueue(message, message->size, Self.OutboundQueue, 1);

	return;
}

void PC_TX(byte Reached_IP[2], byte PBID[2], byte SNR)
{
	byte* PCPacket = (byte*)malloc(sizeof(byte)*8);

    if(PCPacket == NULL)
    {
        fatalErr("Couldn't assign memory to PB Packet \n");
    }

    PCPacket[0]=(PROTOCOL_VERSION<<4)+PC;
    PCPacket[1]=Self.IP[0];
    PCPacket[2]=Self.IP[1];
    PCPacket[3]=Reached_IP[0];
    PCPacket[4]=Reached_IP[1];
    PCPacket[5]=PBID[0];
    PCPacket[6]=PBID[1];
    PCPacket[7]=SNR;

	out_message* message = newOutMessage(getPacketSize(PCPacket), PCPacket);
	addToQueue(message, message->size, Self.OutboundQueue, 1);

	return;
}

void TA_TX(byte Originator_IP[2], byte PBID[2])
{
	return;
}

void TB_TX(byte PBID[2], timetable* tm, in_message* message)
{
	return;
}

void NE_TX(byte Proxy_IP[2])
{
	return;
}

void NEP_TX(byte Outsiders_IP[2])
{
	return;
}

void NER_TX(byte Outsiders_IP[2])
{
	return;
}

void NEA_TX(byte Outsiders_IP[2], byte PBID[2])
{
	return;
}



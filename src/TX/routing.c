#include "routing.h"

void PB_TX(void* packet)
{
	out_message* message = newOutMessage(getPacketSize(packet), packet);
    addToQueue(message, message->size, Self.OutboundQueue, 1);
}


void PR_TX(void* PRPacket)
{
	out_message* message = newOutMessage(getPacketSize(PRPacket), PRPacket);
	addToQueue(message, message->size, Self.OutboundQueue, 1);

	return;
}

void PC_TX(byte Reached_IP[2], byte PBID[2], float PBE)
{
	byte* PCPacket = (byte*)malloc(sizeof(byte)*11);

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
    *((float*)(&(PCPacket[7]))) = PBE;


	out_message* message = newOutMessage(getPacketSize(PCPacket), PCPacket);
	addToQueue(message, message->size, Self.OutboundQueue, 1);

	return;
}


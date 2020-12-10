#include "network.h"

void NE_TX(void* message)
{
	out_message* out_message = newOutMessage(getPacketSize(message), message);
    addToQueue(out_message, out_message->size, Self.OutboundQueue, 1);
}

void NEP_TX(byte Outsiders_IP[2])
{
    out_message* NEPMessage = buildNEPMessage(Self.IP, Outsiders_IP);
    addToQueue(NEPMessage, NEPMessage->size, Self.OutboundQueue, 1);
}

//void NER_TX(byte Outsiders_IP[2])
void NER_TX(void* message)
{
	out_message* out_message = newOutMessage(getPacketSize(message), message);
    addToQueue(out_message, out_message->size, Self.OutboundQueue, 1);

    /*
	byte* NextHopIP = Self.Table->begin->Neigh_IP;
    out_message* NERMessage;
    
    NERMessage = buildNERMessage(NextHopIP, Outsiders_IP);
    addToQueue(NERMessage, NERMessage->size, Self.OutboundQueue, 1);

	return NERMessage;
	*/
}

void NEA_TX(byte Outsiders_IP[2], pbid PBID)
{
    out_message* NEAMessage = buildNEAMessage(Outsiders_IP, PBID);
    addToQueue(NEAMessage, NEAMessage->size, Self.OutboundQueue, 1);
	return;
}



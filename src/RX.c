#include "RX.h"


void*
WF_listener(void* dummy)
{
	char buff[MAXIMUM_PACKET_SIZE*2];		// *2 to prevent overflow on buff+PrevBytes
	int PacketSize;
	int ReadBytes = 0;
	int PrevBytes = 0;
	in_message* message;
	timespec res;

	printf("WF Listener on\n");
	while(1)
	{
		ReadBytes = getFromSocket(Meta.Input_socket, buff+PrevBytes);
		PrevBytes = 0;

		if(clock_gettime(CLOCK_REALTIME, &res) == -1)
		{
			printfErr("Could not clock_gettime\n");
			// Ignore received messages here??? or handle unavailable timestamp elsewhere
			continue;
		}

		PacketSize = getPacketSize(buff);

		if(PacketSize == -1)
		{
			dumpBin(buff, ReadBytes, "Packet size returned -1, dumping buffer\n");
			continue;
		}

		if(PacketSize > ReadBytes)
		{
			// Undefined behaviour, just ignore for now
			// Eventually we could try to receive more and "complete the packet"?
			// This could fail so a "timer" would be needed. Too complex?
			dumpBin(buff, ReadBytes, "Packet size (%d) is more than what was received (%d).\n", PacketSize, ReadBytes);
			continue;
		}

		addToQueue(newInMessage(PacketSize, buff, res), 8, Self.InboundQueue, 1);

		// We received more than one packet
		if(PacketSize < ReadBytes)
		{
			// Copy the last of the read bytes, to the beggining of the buffer
			PrevBytes = PacketSize;
			for(int i = 0; PacketSize+i < ReadBytes; PacketSize++, i++)
			{
				buff[i] = buff[PacketSize+i];
			}
		}

		sleep(1);	// For testing purposes
	}
}

in_message*
getMessage()
{
	return NULL;
}

void*
WS_listener(void* dummy)
{
	printf("WS Listener on\n");
	while(1)
    {
        sleep(1);
    }
}

void SD_RX(in_message* msg)
{
	return;
}

void PB_RX(in_message* msg)
{
	byte senderIp[2];
	byte PBID[2];
	senderIp[0]=((byte*)msg->buf)[1];
	senderIp[1]=((byte*)msg->buf)[2];
	PBID[0]=((byte *)msg->buf)[3];
	PBID[1]=((byte *)msg->buf)[4];
	unsigned short distance	=(((byte *)msg->buf)[5]<<8) + ((byte *)msg->buf)[6];

	if(Self.Status == Outside){ //if the node is an outside slave 

		if(distance!= (unsigned short)65535)
		{
			NE_TX(senderIp);//sends a NE
			//DO I NEED TO DEAL WITH TIMEOUT RETRANSMISSION HERE?
			return; 
		}	
	}
	/*else
	{
		/*if(pibdSearchPair(senderIp,PBID,Self.PB_PBIDTable;)==0)
		{
			pbidInsertPair(senderIP,PBID,Self.PB_PBIDTable;); //stores pair in PBID table
			PR_TX(senderIp, PBID, msg->SNR);
			//sets timeout FOR RETRANSMISSION
		}
	}

	delInMessage(msg);
	return;*/
}

void PR_RX(in_message* msg)
{
	byte SenderIp[2];
	byte originatorIp[2];
	byte PBID[2];
	SenderIp[0]=((byte*)msg->buf)[1];
	SenderIp[1]=((byte*)msg->buf)[2];
	originatorIp[0]=((byte*)msg->buf)[3];
	originatorIp[1]=((byte*)msg->buf)[4];
	PBID[0]=((byte *)msg->buf)[5];
	PBID[1]=((byte *)msg->buf)[6];

	unsigned short distance =(((byte *)msg->buf)[7]<<8) + ((byte *)msg->buf)[8];

	if(originatorIp[0]== Self.IP[0] && originatorIp[1]== Self.IP[1]) //the node is receiving a PR from a PB it generated
	{
		routInsertOrUpdateEntry(Self.Table, SenderIp, distance, msg->SNR, 0);
		PC_TX(SenderIp,PBID,msg->SNR);
		Self.RoutingPBID++;
	}
	
	delInMessage(msg);
	return;
}

void PC_RX(in_message* msg)
{

	byte SenderIP[2];
	byte ReachedIP[2];
	SenderIP[0]=((byte*)msg->buf)[1];
	SenderIP[1]=((byte*)msg->buf)[2];
	ReachedIP[0]=((byte*)msg->buf)[3];
	ReachedIP[1]=((byte*)msg->buf)[4];

	if(ReachedIP[0]== Self.IP[0] && ReachedIP[1]== Self.IP[1])
	{
		//routInsertOrUpdateEntry(Self.Table, SenderIP, distance, msg->SNR, 0); gotta check the distance with Bruno M
		//stop retransmissions 
	}	
	
	delInMessage(msg);
	return;
}

void TA_RX(in_message* msg)
{
	return;
}

void TB_RX(in_message* msg)
{
	return;
}

void NE_RX(in_message* msg)
{
	return;
}

void NEP_RX(in_message* msg)
{
	return;
}

void NER_RX(in_message* msg)
{
	return;
}

void NEA_RX(in_message* msg)
{
	return;
}

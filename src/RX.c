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
	byte sendergitIp[2];
	senderIp[0]=((byte*)msg->buf)[1];
	senderIp[1]=((byte*)msg->buf)[2];
	short pbid=(((byte *)msg->buf)[3]<<8) + ((byte *)msg->buf)[4];
	short distance	=(((short *)msg->buf)[5]<<8) + ((short *)msg->buf)[6];

	if(Self.Status == Outside){ //if the node is an outside slave 

		if(distance!= 65535)
		{
			NE_TX(senderIp);//sends a NE
			//DO I NEED TO DEAL WITH TIMEOUT RETRANSMISSION HERE?
			return; 
		}	
	}
	else
	{
		/*if(truewe already delt with this pb))*/
	}
	
	
	//delInMessage(msg);
	return;
}

void PR_RX(in_message* msg)
{
	return;
}

void PC_RX(in_message* msg)
{
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

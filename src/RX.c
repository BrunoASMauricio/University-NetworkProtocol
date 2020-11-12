#include "RX.h"


void*
WF_listener(void* dummy)
{
	char buff[MAXIMUM_PACKET_SIZE*2];		// *2 to prevent overflow on buff+PrevBytes
	int PacketSize;
	int ReadBytes = 0;
	int PrevBytes = 0;
	unsigned int received_messages = 0;
	in_message* message;
	timespec res;

	printf("WF Listener on port %u\n", Meta.WF_RX->port);

	// This is needed to kickstart the connection
	// Everywhere I looked, client always spoke first
	// Always found "in UDP it doesn't matter who talks first"
	// But if client doesn't send this ping, it doesn't work
	sleep(1);
	sendToSocket(Meta.WF_RX, &PrevBytes, 2);

	PrevBytes = 0;
	while(1)
	{
		while((ReadBytes = getFromSocket(Meta.WF_RX, buff+PrevBytes)) == -1)
		{
			continue;
		}
		printf("\t\t-------Node got packet (%d bytes) total of %d!!-------\n", ReadBytes, ++received_messages);

		if(((byte*)buff)[0] & 0x0f == TB && ReadBytes < 18)
		{
			PrevBytes = ReadBytes;
			printf("Got truncated TB\n");
			continue;
		}

		clock_gettime(CLOCK_REALTIME, &res);

		PacketSize = getPacketSize(buff);

		if(PacketSize == -1)
		{
			dumpBin(buff, ReadBytes, "Packet size returned -1, dumping buffer\n");
			continue;
		}

		if(PacketSize > ReadBytes + PrevBytes)
		{
			dumpBin(buff, ReadBytes, "Packet size (%d) is more than what was received (%d).\n", PacketSize, ReadBytes + PrevBytes);
			PrevBytes = ReadBytes;
			continue;
		}

		addToQueue(newInMessage(PacketSize, buff, res), 8, Self.InboundQueue, 1);

		// We received more than one packet
		if(PacketSize < ReadBytes + PrevBytes)
		{
			printf("\t\tMore than one packet (%d, %d)\n", PacketSize, ReadBytes+PrevBytes);
			// Copy the last of the read bytes, to the beggining of the buffer
			for(int i = 0; PacketSize + i < ReadBytes + PrevBytes; i++)
			{
				buff[i] = buff[PacketSize+i];
			}
			PrevBytes = PacketSize - (PrevBytes + ReadBytes);
		}
		else
		{
			PrevBytes = 0;
		}
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
	void* buff = msg->buf;
	bool send_TA = false;
	bool retransmit_TB = false;
	byte* local_byte;
	int ip_amm;
	byte slot;
	// New PBID? Accept new timeslot
	
	pthread_mutex_lock(&(Self.TimeTable->Lock));
	if(((byte*)buff)[0] != Self.TB_PBID[0] && ((byte*)buff)[1] == Self.TB_PBID[1])
	{
		Self.TimeTable->local_slot = -1;
		Self.TimeTable->table_size = ((short*)(((byte*)buff+16)))[0];
		ip_amm = Self.TimeTable->table_size;
		for(int i = 0; i < ip_amm; i++)
		{
			if(((short*)(((byte*)buff+18)))[i] == ((short*)Self.IP)[0])
			{
				Self.TimeTable->local_slot = i;
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

	for(int i = 0; i < Self.SubSlaves->L->Size; i++)
	{
		retransmit_TB |= getBitmapValue(getIPFromList(Self.SubSlaves, i), (byte*)buff+18+ip_amm*2, ip_amm, (byte*)buff+18);
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



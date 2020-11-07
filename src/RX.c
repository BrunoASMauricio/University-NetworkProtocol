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
		while((ReadBytes = getFromSocket(Meta.WF_RX, buff+PrevBytes)) == -1)
		{
			continue;
		}
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
/*
	((byte*)buff)[0] = (0xff00 & (PROTOCOL_VERSION<<4)) | TB;
	((byte*)buff)[1] = Self.IP[0];
	((byte*)buff)[2] = Self.IP[1];
	((byte*)buff)[3] = Self.TB_PBID[0];
	((byte*)buff)[4] = Self.TB_PBID[1];
	((unsigned long int*)((byte*)buff+5))[0] = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;
	((short*)(((byte*)buff+13)))[0] = DEFAULT_VALIDITY_DELAY;
	(((byte*)buff+15))[0] = DEFAULT_TIMESLOT_SIZE;
	((short*)(((byte*)buff+16)))[0] = ip_amm;
	for(int i = 0; i < ip_amm; i++)
	{
		((short*)(((byte*)buff+18)))[i] = ((short*)getIPFromList(Self.SubSlaves, i))[0];
	}

	for(int i = 0; i < ip_amm/8; i++)
	{
		((byte*)buff)[16+ip_amm*2+i] = 0xff;
	}
	rest = ip_amm - 8*(ip_amm/8);
	if(rest)
	{
		// No point in "cutting" the last bits, because the bitmap must
		// already cut them
		((byte*)buff)[16+ip_amm*2+(ip_amm/8)] = (0xff<<(8-rest);
	}

 */

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
		Self.TimeTable->Local_slot = -1;
		Self.TimeTable->Table_size = ((short*)(((byte*)buff+16)))[0];
		ip_amm = Self.TimeTable->Table_size;
		for(int i = 0; i < ip_amm; i++)
		{
			if(((short*)(((byte*)buff+18)))[i] == ((short*)Self.IP)[0])
			{
				Self.TimeTable->Local_slot = i;
				break;
			}
		}
		if(Self.TimeTable->Local_slot == -1)
		{
			dumpBin((char*)buff, getPacketSize(buff), "Did not receive timeslot from TB\n");
			// SET STATE TO OUTSIDE NETWORK
			return;
		}
		Self.TimeTable->Timeslot_size = (((byte*)buff+15))[0];
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



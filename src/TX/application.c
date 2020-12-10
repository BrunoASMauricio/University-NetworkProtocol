#include "application.h"

void SD_TX(void* buff, int size)
{
	int TotalSize=0, NumSamples=0, Popped;

	byte packet[Packet_Sizes[SD]+MAX_PAYLOAD_SIZE];
   	//	= (byte*)malloc(sizeof(byte)*(Packet_Sizes[SD] + size));
	byte* NextHopIP = Self.Table->begin->Neigh_IP;
	//Assuming first position in table is itself; otherwise, search in table by self IP or something else

	if (NextHopIP == NULL)
	{
		printf("Next hop still undefined, dropping data message\n");
        return;
	}

	packet[0] = (PROTOCOL_VERSION<<4) + SD;
	packet[1] = Self.IP[0];
	packet[2] = Self.IP[1];
	packet[3] = NextHopIP[0];
	packet[4] = NextHopIP[1];
	// To be replaced by Seq and TTL(set to a max of 8)
	packet[5] = (0<<4) + 8;
	packet[6] = size;

	memcpy(&packet[7], buff, size);
	addToQueue(newOutMessage(getPacketSize(packet), packet), getPacketSize(packet), Self.OutboundQueue, 1);
}


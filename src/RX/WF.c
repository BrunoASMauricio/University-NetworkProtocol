#include "WF.h"

void
WF_listener()
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
		printf("Got %d bytes from socket\n", ReadBytes);
		if(((byte*)buff)[0] & 0x0f == TB && ReadBytes < 18)
		{
			PrevBytes = ReadBytes;
			printf("Got truncated TB\n");
			continue;
		}

		clock_gettime(CLOCK_REALTIME, &res);

		PacketSize = getPacketSize(buff) + 4; // Also get SNR for the message

		if(PacketSize == -1)
		{
			dumpBin(buff, ReadBytes+PrevBytes, "Packet size returned -1, dumping buffer\n");
			continue;
		}

		if(PacketSize > ReadBytes + PrevBytes)
		{
			dumpBin(buff, ReadBytes, "Packet size (%d) is more than what was received (%d).\n", PacketSize, ReadBytes + PrevBytes);
			PrevBytes = ReadBytes;
			continue;
		}
		printf("\t\t-------Node got message (%d/%d bytes) total of %d!!-------\n", ReadBytes, ReadBytes+PrevBytes, ++received_messages);


		message = newInMessage(PacketSize-4, buff, res);
		message->PBE = ((float*)(buff + PacketSize-4))[0];
		printf("Received full correct message! Received SNR: %u\n", message->PBE);
		printMessage(buff, PacketSize);
		// Directly handle message
		handler(message);

		//addToQueue(newInMessage(PacketSize, buff, res), 8, Self.InboundQueue, 1);

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



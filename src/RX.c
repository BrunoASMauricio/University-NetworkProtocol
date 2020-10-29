#include "RX.h"


void*
WF_listener(void* dummy)
{
	char buff[MAXIMUM_PACKET_SIZE*2];		// *2 to prevent overflow on buff+prev_bytes
	int packet_size;
	int read_bytes = 0;
	int prev_bytes = 0;
	in_message* message;
	timespec res;

	printf("WF Listener on\n");
	while(1){
		read_bytes = getFromSocket(Meta.Input_socket, buff+prev_bytes);
		prev_bytes = 0;

		if(clock_gettime(CLOCK_REALTIME, &res) == -1)
		{
			printfErr("Could not clock_gettime\n");
			// Ignore received messages here??? or handle unavailable timestamp elsewhere
			continue;
		}

		packet_size = getPacketSize(buff);
		if(packet_size == -1)
		{
			dumpBin(buff, read_bytes, "Packet size returned -1, dumping buffer\n");
			continue;
		}

		if(packet_size > read_bytes)
		{
			// Undefined behaviour, just ignore for now
			// Eventually we could try to receive more and "complete the packet"?
			// This could fail so a "timer" would be needed. Too complex?
			dumpBin(buff, read_bytes, "Packet size (%d) is more than what was received (%d).\n", packet_size, read_bytes);
			continue;
		}

		addToQueue(newMessage(packet_size, buff, res), 8, Self.InboundQueue, 1);

		// We received more than one packet
		if(packet_size < read_bytes)
		{
			// Copy the last of the read bytes, to the beggining of the buffer
			prev_bytes = packet_size;
			for(int i = 0; packet_size+i < read_bytes; packet_size++, i++)
			{
				buff[i] = buff[packet_size+i];
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



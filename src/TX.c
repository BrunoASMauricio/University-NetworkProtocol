#include "TX.h"
#include "data.h"

void*
WF_dispatcher(void* dummy)
{
	unsigned long int Act;
	unsigned long int Vact;
	unsigned long int Next;
	unsigned long int Slot;
	unsigned int sent_messages = 0;
	out_message* To_send;
	timespec Res;
	int Size;
	printf("WF Dispatcher on %u\n", Meta.WF_TX->port);
	while(1)
    {
		// Is there still a message to send? If not wait for one
		if(To_send == NULL)
		{
			while(!(To_send = (out_message*)popFromQueue(&Size, Self.OutboundQueue)))
			{
				usleep(TX_MESSAGE_WAIT);
			}
		}
		// Not on the network, just send it
		if(Self.TimeTable->sync == 0)
		{
			while(sendToSocket(Meta.WF_TX, To_send->buf, To_send->size) == -1)
			{
				continue;
			}
			if(Self.SyncTimestamp)
			{
				clock_gettime(CLOCK_REALTIME, &Res);
				Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
				while(sendToSocket(Meta.WF_TX, &Act, 8) == -1)
				{
					continue;
				}
			}
			printf("Message sent! total of %d\n", ++sent_messages);
			dumpBin((char*)(To_send->buf), To_send->size, "SENT PACKET!: ");
			delOutMessage(To_send);
			To_send = NULL;
		}
		else
		{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			Slot = Self.TimeTable->sync + Self.TimeTable->local_slot * Self.TimeTable->timeslot_size;
			Vact = Act - Slot;
			Next = Self.TimeTable->table_size * ((Vact/Self.TimeTable->table_size) + 1) + Slot;
			
			// Timetable isn't yet valid or first timeslot hasn't elapsed
			if(Act < Self.TimeTable->sync || Act < Slot)
			{
				continue;
			}

			if (Vact < Self.TimeTable->table_size * (Vact / Self.TimeTable->table_size) + Self.TimeTable->timeslot_size - TRANSMISSION_DELAY)
			{
				//printf("In timeslot: %lu\n", act);
				while(sendToSocket(Meta.WF_TX, To_send->buf, To_send->size) == -1)
				{
					continue;
				}
				if(Self.SyncTimestamp)
				{
					while(sendToSocket(Meta.WF_TX, &Act, 8) == -1)
					{
						continue;
					}
				}
				delOutMessage(To_send);
				To_send = NULL;
			}
			else
			{
				// Tried to make the thread sleep precisely, but failed miserably (for now)
				//clock_gettime(CLOCK_REALTIME, &res);
				//act = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;
				//printf("Sleeping for %ld", (next-act)/100UL);
				//usleep((next-act)/1000UL);
			}
		}
        //sleep(1);
    }
}

void*
HW_dispatcher(void*dummy)
{
     printf("HW Dispatcher on\n");
    
    int PacketSize=0;  
    byte *Popped = (byte*)malloc(sizeof(byte)*(TOTALLENGTH));  


    socket_s* sockfd = newSocket(PORTHW);
    startSocket(sockfd);

	if(!Self.IsMaster)
	{
		// Slaves don't require HW dispatcher
		return NULL;
	}
    while (1)
    {
        //  Sending========>    IP     DataPayload      // 
       //                       2bytes 3bytes          //  possible changes of these values // 
        
		Popped = (byte*) popFromQueue(&PacketSize,Self.InternalQueue);
		
		if (Popped == NULL) PacketSize = 0;

		if (PacketSize > 0)
		{
			dumpBin((char*)Popped, PacketSize, "Sending to HW (%d bytes): ", PacketSize);
			if(Self.IsMaster)
			{
				//printf("SENT MESSAGE TO HW %d\n", PacketSize);
			}
			sendToSocket(sockfd, Popped ,sizeof(byte)*PacketSize);
		}

		PacketSize=0;
		usleep(HW_DISPATCHER_SLEEP);
    }
    free(Popped); 
    close(sockfd->s); 
}




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

void* createPB()
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
    PBPacket[3]= (getNewPBID()>> 8) &0xff;
    PBPacket[4]= Self.PBID &0xff ;

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
	return PBPacket;
}
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

    out_message *TAMessage = buildTAMessage(Originator_IP, PBID);
    addToQueue(TAMessage, TAMessage->size, Self.OutboundQueue, 1);
    return;

}


void TB_TX(void* message)
{
	out_message* out_message = newOutMessage(getPacketSize(message), message);
    addToQueue(out_message, out_message->size, Self.OutboundQueue, 1);
}

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



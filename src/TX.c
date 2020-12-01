#include "TX.h"


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
	printf("WF Dispatcher on\n");
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
		if(Self.TimeTable == NULL)
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

void
sendMessage(void* msg)
{

}

void*
HW_dispatcher(void*dummy)
{
     printf("HW Dispatcher on\n");
    
    int PacketSize=0;  
    byte *Popped = (byte*)malloc(sizeof(byte)*(TOTALLENGTH));  


    socket_s* sockfd = newSocket(PORTHW);
    startSocket(sockfd);

    while (1)
    {
        //  Sending========>    IP     DataPayload      // 
       //                       2bytes 3bytes          //  possible changes of these values // 
        
        if( Self.IsMaster == 1)
        {
            Popped = (byte*) popFromQueue(&PacketSize,Self.InternalQueue);
            
            if (Popped == NULL) PacketSize = 0;

            if (PacketSize > 0)
            {
               sendToSocket(sockfd, Popped ,sizeof(byte)*PacketSize); 
            }

            PacketSize=0; 
        } 
    }
    free(Popped); 
    close(sockfd->s); 
}




void SD_TX(int Sample_Ammount)
{
	if (Self.InternalQueue->First == NULL) 
	{
		return;
	}

	int TotalSize=0, NumSamples=0, Popped;

	if(Self.InternalQueue->First->PacketSize < Sample_Ammount)
	{
		NumSamples = Self.InternalQueue->First->PacketSize;
	}
	else
	{
		NumSamples = Sample_Ammount;
	}
	if(NumSamples>255)
	{
		printf("SD can't read more than 255 samples (tried to read %d)\n", NumSamples);
		NumSamples = 255;
	}

	byte* packet = (byte*)malloc(sizeof(byte)*(Packet_Sizes[1] + NumSamples)), *Data;
	byte* NextHopIP = Self.Table->begin->Neigh_IP; //Assuming first position in table is itself; otherwise, search in table by self IP or something else
	if (NextHopIP == NULL)
	{
        printf("Next hop still undefined.\n");
        return;
		//Comment return and uncomment these lines for testing purposes
		//NextHopIP[0] = 0;
		//NextHopIP[1] = 0;
	}

	packet[0] = (PROTOCOL_VERSION<<4) + SD;
	packet[1] = Self.IP[0];
	packet[2] = Self.IP[1];
	packet[3] = NextHopIP[0];
	packet[4] = NextHopIP[1];
	packet[5] = (0<<4) + 8;		// To be replaced by Seq and TTL(set to a max of 8)
	packet[6] = NumSamples;

	Data = (byte*) popFromQueue(&Popped, Self.InternalQueue);

	memcpy(&packet[7], &Data, NumSamples);
	
	addToQueue(newOutMessage(sizeof(byte)*(Packet_Sizes[1] + NumSamples), packet), Packet_Sizes[1] + NumSamples, Self.OutboundQueue, 1);
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
    PBPacket[3]= (Self.RoutingPBID >> 8) &0xff;
    PBPacket[4]= Self.RoutingPBID &0xff ;

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

void PR_TX(byte Originator_IP[2], byte PBID[2], byte SNR)
{
	byte* PRPacket = (byte*)malloc(sizeof(byte)*10);

    if(PRPacket == NULL)
    {
        fatalErr("Couldn't assign memory to PB Packet \n");
    }

    table_entry* FirstEntry = routGetEntryByPos(Self.Table, 1);
    PRPacket[0]=(PROTOCOL_VERSION<<4)+PR;
    PRPacket[1]=Self.IP[0];
    PRPacket[2]=Self.IP[1];
    PRPacket[3]=Originator_IP[0];
    PRPacket[4]=Originator_IP[1];
    PRPacket[5]=PBID[0];
    PRPacket[6]=PBID[1];
    PRPacket[7]=(FirstEntry->Distance >> 8) &0xff;
    PRPacket[8]=FirstEntry->Distance &0xff;
    PRPacket[9]=SNR;

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
	return;
}

void TB_TX(byte PBID[2], void* buff)
{
	// Lock TB_PBID table
	// Lock SubSlaves
	addToQueue(newOutMessage(getPacketSize(buff), buff), 8, Self.OutboundQueue, 1);
	
	return;
}

void NE_TX(byte Proxy_IP[2])
{
	return;
}

void NEP_TX(byte Outsiders_IP[2])
{
	return;
}

void NER_TX(byte Outsiders_IP[2])
{
	return;
}

void NEA_TX(byte Outsiders_IP[2], byte PBID[2])
{
	return;
}



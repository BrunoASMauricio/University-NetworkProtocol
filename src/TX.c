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
				// usleep(10);
				continue;
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
    byte *Popped = (byte*)malloc(sizeof(byte)*(IPLENGTH+TIMESTAMPLENGTH+(SAMPLELENGTH*NUMSAMPLES)));  
    byte *ip = (byte*)malloc(sizeof(byte)*IPLENGTH);
    byte *TimeStamp = (byte*)malloc(sizeof(byte)*TIMESTAMPLENGTH);
    byte *Sample = (byte*)malloc(sizeof(byte)*SAMPLELENGTH);
    byte *TotalSample = (byte*)malloc(sizeof(byte)*TOTALSAMPLELENGTH);

    socket_s* sockfd = newSocket(PORTHW);
    startSocket(sockfd);

    while (1)
    {
        //  Sending========>    IP     TimeStamp Sample      // 
       //                       2bytes 2bytes    2bytes     //  possible changes of these values // 
      //
     //     Note: add 50ms to each timestamp then send to HW           
        
        if( Self.IsMaster == 1)
        {
            Popped = (byte*) popFromQueue(&PacketSize,Self.InternalQueue);
            
            if (Popped == NULL) PacketSize = 0;
            if (PacketSize > 0 )
            {
                
                memcpy(TotalSample, Popped, IPLENGTH);
                memcpy(TotalSample+IPLENGTH, Popped+IPLENGTH, TIMESTAMPLENGTH);

                for(int i=0; i<NUMSAMPLES;i++)
                {
                    memcpy(TotalSample + IPLENGTH + TIMESTAMPLENGTH, 
                            Popped + IPLENGTH + TIMESTAMPLENGTH + SAMPLELENGTH*i, 
                                SAMPLELENGTH);
                    
                    sendToSocket(sockfd, TotalSample ,sizeof(byte)*TOTALSAMPLELENGTH); 
                }

                // LAST THING TO DO: after sending the first set of Samples will be incremented 50ms (1/20) to the next Samples on timestamp //
               //                                                                    and will always repeat this for the next N samples         //

            }
            PacketSize=0; 
        } 
        else 
        {
            printf(" \n I'm a Slave. Nothing to do here.\n");
        }
    }
    free(Popped);
    free(ip);   
    free(TimeStamp);   
    free(Sample); 
    free(TotalSample);    
    close(sockfd->port); 
}




void SD_TX(int Sample_Ammount)
{
	if (Self.InternalQueue->First == NULL) 
	{
		return;
	}

	int TotalSize=0, NumSamples=0, Popped;
	queue_el* AuxEl = Self.InternalQueue->First;


	if(AuxEl->PacketSize < Sample_Ammount)
	{
		NumSamples = AuxEl->PacketSize;
	}
	else
	{
		NumSamples = Sample_Ammount;
	}

	byte* packet = (byte*)malloc(sizeof(byte)*(7+ NumSamples)), *Data;
	byte* NextHopIP = Self.Table->begin->Neigh_IP; //Assuming first position in table is itself; otherwise, search in table by self IP or something else
	if (NextHopIP == NULL)
	{
        printf("Next hop still undefined.\n");
        return;
		//Comment return and uncomment these lines for testing purposes
		//NextHopIP[0] = 0;
		//NextHopIP[1] = 0;
	}

	packet[0] = (PROTOCOL_VERSION<<4) + 1;
	packet[1] = Self.IP[0];
	packet[2] = Self.IP[1];
	packet[3] = NextHopIP[0];
	packet[4] = NextHopIP[1];
	packet[5] = (0<<4) + 8;		// To be replaced by Seq and TTL(set to a max of 8)
	packet[6] = NumSamples;

	Data = (byte*) AuxEl->Packet;
	for (int i = 0; i < NumSamples; i++)
	{
		packet[7+i] = Data[i];
	}
	popFromQueue(&Popped, Self.InternalQueue);
	addToQueue(newOutMessage(sizeof(byte)*(7+ NumSamples), packet), 8, Self.OutboundQueue, 1);
}

void PB_TX(byte PBID[2])
{
	return;
}

void PR_TX(byte Originator_IP[2], byte PBID[2], byte SNR)
{
	return;
}

void PC_TX(byte Reached_IP, byte PBID[2], byte SNR)
{
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



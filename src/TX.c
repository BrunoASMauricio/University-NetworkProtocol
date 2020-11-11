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
    byte *Popped = (byte*)malloc(sizeof(byte)*(IPLENGTH+DATAPAYLOADLENGTH));  
    byte *ToSend = (byte*)malloc(sizeof(byte)*(TOTALLENGTH)); 

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
               memcpy(ToSend,Popped, TOTALLENGTH);

               sendToSocket(sockfd, ToSend ,sizeof(byte)*TOTALLENGTH); 
            }

            PacketSize=0; 
        } 
    }
    free(Popped); 
    free(ToSend); 
    close(sockfd->s); 
}




void SD_TX(int Sample_Ammount, void* Samples)
{
	return;
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



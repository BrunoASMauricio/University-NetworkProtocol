#include "TX.h"


void*
WF_dispatcher(void* dummy)
{
	unsigned long int Act;
	unsigned long int Vact;
	unsigned long int Next;
	unsigned long int Slot;
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
			sendToSocket(Meta.WF_TX, To_send->buf, To_send->size);
			delOutMessage(To_send);
			To_send = NULL;
		}
		else
		{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			Slot = Self.TimeTable->sync + Self.TimeTable->local * Self.TimeTable->timeslot_size;
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
				sendToSocket(Meta.WF_TX, To_send->buf, To_send->size);
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
        sleep(1);
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

void TB_TX(byte PBID[2], timetable* tm, in_message* message)
{
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



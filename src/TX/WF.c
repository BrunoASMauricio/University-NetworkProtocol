#include "WF.h"


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
			printMessage(To_send->buf, To_send->size);
			//dumpBin((char*)(To_send->buf), To_send->size, "SENT PACKET!: ");
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


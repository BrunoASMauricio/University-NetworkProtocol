#include "WF.h"


void*
WF_dispatcher(void* dummy)
{
	unsigned long int Act;
	unsigned long int Vact;
	unsigned long int Next;
	unsigned long int Slot;
	unsigned long int Sleep;
	unsigned int message_size;
	unsigned int sent_messages = 0;
	int n;

	out_message* To_send = NULL;
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
			message_size = getPacketSize(To_send->buf);
		}
		clock_gettime(CLOCK_REALTIME, &Res);
		Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
		checkNewTimeTable(Act);
		// Not on the network, just send it
		if(Self.TimeTable->sync == 0)
		{
			printf("Not on the network, blind transmition\n");
			while((n=sendToSocket(Meta.WF_TX, To_send->buf, message_size)) == -1)
			{
				continue;
			}
			if(Self.SyncTimestamp)
			{
				clock_gettime(CLOCK_REALTIME, &Res);
				Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
				while((n=sendToSocket(Meta.WF_TX, &Act, 8)) == -1)
				{
					continue;
				}
			}
			printf("Message sent! total of %d\n", ++sent_messages);
			printMessage(To_send->buf, message_size);
			//dumpBin((char*)(To_send->buf), To_send->size, "SENT PACKET!: ");
			delOutMessage(To_send);
			To_send = NULL;
		}
		else
		{
			Slot = Self.TimeTable->sync + Self.TimeTable->local_slot * Self.TimeTable->timeslot_size;
			Vact = Act - Slot;
			Next = Self.TimeTable->table_size * ((Vact/Self.TimeTable->table_size) + 1) + Slot;
			// Timetable isn't yet valid or first timeslot hasn't elapsed
			if(Act < Self.TimeTable->sync || Act < Slot)
			{
				// Sleep until sync
				// This sleep may slightly exceed what is requested
				if(Act < Self.TimeTable->sync)
				{
					printf("Sleeping for %lu us\n", (Self.TimeTable->sync-Act)/1E3);
					usleep((Self.TimeTable->sync-Act)/1E3);
				}
				else
				{
					printf("Sleeping for %lu us\n", (Slot-Act)/1E3);
					usleep((Slot-Act)/1E3);
				}
			}
			else
			{
				if (Vact < Self.TimeTable->table_size * (Vact / Self.TimeTable->table_size) + Self.TimeTable->timeslot_size - TRANSMISSION_JITTER/*TRANSMISSION_DELAY*8*message_size*/)
				{
					while((n=sendToSocket(Meta.WF_TX, To_send->buf, message_size)) == -1)
					{
						continue;
					}
					if(Self.SyncTimestamp)
					{
						clock_gettime(CLOCK_REALTIME, &Res);
						Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
						while((n=sendToSocket(Meta.WF_TX, &Act, 8)) == -1)
						{
							continue;
						}
					}
					printf("Message sent! total of %d size :%d %lu\n", ++sent_messages, message_size, Act);
					printMessage(To_send->buf, message_size);
					delOutMessage(To_send);
					To_send = NULL;
				}
				else
				{
					printf("Timetable info:\n");
					printf("Sync: %lu\n", Self.TimeTable->sync);
					printf("timeslot: %lu %lu\n", Self.TimeTable->timeslot_size, Slot);
					printf("timetable size: %lu, ammount of elapsed %lu\n",Self.TimeTable->table_size,((Vact/(Self.TimeTable->table_size)) + 1));
					printf("In timeslot: %lu, from %lu to %lu\n", Act, Next-Self.TimeTable->table_size, Next);
					// Tried to make the thread sleep precisely, but failed miserably (for now)
					//clock_gettime(CLOCK_REALTIME, &res);
					//act = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;
					//printf("Sleeping for %ld", (next-act)/100UL);
					//usleep((next-act)/1000UL);
					Sleep = (Next - Act)/1E3;
					printf("Sleeping for %lu us\n", Sleep);
					fflush(stdout);
					usleep(Sleep);
				}
			}
		}
        //sleep(1);
    }
}


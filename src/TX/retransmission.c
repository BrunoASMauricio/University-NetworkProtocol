#include "retransmission.h"


void startRetransmission(retransmitable message_type, void* msg)
{
	timespec Res;
	unsigned long int Act;
	pthread_mutex_lock(&(Self.Rt.Lock));

	SETBIT(message_type, Self.Rt.Retransmitables);

	printf("Starting retransmission of %d\n", message_type);
	
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
	switch(message_type){
		case rPB:
			Self.Rt.Time_PB = Act + RETRANSMISSION_DELAY_PB;
			Self.Rt.PB_ret_msg = msg;
			Self.Rt.PB_ret_amm = 0;
			break;
		case rTB:
			Self.Rt.Time_TB = Act + RETRANSMISSION_DELAY_TB;
			Self.Rt.TB_ret_msg = msg;
			Self.Rt.TB_ret_amm = 0;
			break;
		case rPR:
			Self.Rt.Time_PR = Act + RETRANSMISSION_DELAY_PR;
			Self.Rt.PR_ret_msg = msg;
			Self.Rt.PR_ret_amm = 0;
			break;
		case rNE:
			Self.Rt.Time_NE = Act + RETRANSMISSION_DELAY_NE;
			Self.Rt.NE_ret_msg = msg;
			Self.Rt.NE_ret_amm = 0;
			break;
		case rNER:
			Self.Rt.Time_NER = Act + RETRANSMISSION_DELAY_NER;
			Self.Rt.NER_ret_msg = msg;
			Self.Rt.NER_ret_amm = 0;
			break;
	}
	
	pthread_mutex_unlock(&(Self.Rt.Lock));
}
void stopRetransmission(retransmitable message_type)
{
	printf("Stopping retransmission of %d\n", message_type);
	pthread_mutex_lock(&(Self.Rt.Lock));
	CLEARBIT(message_type, Self.Rt.Retransmitables);
	switch(message_type){
		case rPB:
			Self.Rt.PB_ret_msg = NULL;
			break;
		case rTB:
			Self.Rt.TB_ret_msg = NULL;
			break;
		case rPR:
			Self.Rt.PR_ret_msg = NULL;
			break;
		case rNE:
			Self.Rt.NE_ret_msg = NULL;
			break;
		case rNER:
			Self.Rt.NER_ret_msg = NULL;
			break;
	}
	pthread_mutex_unlock(&(Self.Rt.Lock));
}
void* retransmit(void* dummy)
{
	unsigned long int earliest;
	unsigned long int Act;
	timespec Res;
	printf("Retransmission thread on\n");
	while(1)
	{
		clock_gettime(CLOCK_REALTIME, &Res);
		Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
		earliest = Act + DEFAULT_RETRANSMIT_CHECK;

		pthread_mutex_lock(&(Self.Rt.Lock));
		
		if(CHECKBIT(rPB, Self.Rt.Retransmitables))
		{
			if(Act > Self.Rt.Time_PB)
			{
				// Transmit PB
				printf("Retransmitting a PB %p\n",Self.Rt.PB_ret_msg);
				dumpBin((char*)(Self.Rt.PB_ret_msg), 7, ">>>");
				addToQueue(newOutMessage(getPacketSize(Self.Rt.PB_ret_msg), Self.Rt.PB_ret_msg), 8, Self.OutboundQueue, 1);
				Self.Rt.PB_ret_amm += 1;
				Self.Rt.Time_PB += RETRANSMISSION_DELAY_PB;
			}
			else if(Self.Rt.Time_PB < earliest)
			{
				earliest = Self.Rt.Time_PB;
			}
		}
		if(CHECKBIT(rTB, Self.Rt.Retransmitables))
		{
			if(Act > Self.Rt.Time_TB)
			{
				// Transmit TB
				printf("Retransmitting a TB\n");
				addToQueue(newOutMessage(getPacketSize(Self.Rt.TB_ret_msg), Self.Rt.TB_ret_msg), 8, Self.OutboundQueue, 1);
				Self.Rt.TB_ret_amm += 1;
				Self.Rt.Time_TB += RETRANSMISSION_DELAY_TB;
			}
			else if(Self.Rt.Time_TB < earliest)
			{
				earliest = Self.Rt.Time_TB;
			}
		}
		if(CHECKBIT(rPR, Self.Rt.Retransmitables))
		{
			if(Act > Self.Rt.Time_PR)
			{
				// Transmit PR
				printf("Retransmitting a PR\n");
				addToQueue(newOutMessage(getPacketSize(Self.Rt.PR_ret_msg), Self.Rt.PR_ret_msg), 8, Self.OutboundQueue, 1);
				Self.Rt.PR_ret_amm += 1;
				Self.Rt.Time_PR += RETRANSMISSION_DELAY_PR;
			}
			else if(Self.Rt.Time_PR < earliest)
			{
				earliest = Self.Rt.Time_PR;
			}
		}
		if(CHECKBIT(rNE, Self.Rt.Retransmitables))
		{
			if(Act > Self.Rt.Time_NE)
			{
				// Transmit NE
				printf("Retransmitting an NE\n");
				addToQueue(newOutMessage(getPacketSize(Self.Rt.NE_ret_msg), Self.Rt.NE_ret_msg), 8, Self.OutboundQueue, 1);
				Self.Rt.NE_ret_amm += 1;
				Self.Rt.Time_NE += RETRANSMISSION_DELAY_NE;
				if(Self.Rt.NE_ret_amm == RETRANSMISSION_ATTEMPTS_NE)
				{
					pthread_mutex_unlock(&(Self.Rt.Lock));
					stopRetransmission(rNE);
					pthread_mutex_lock(&(Self.Rt.Lock));
				}
			}
			else if(Self.Rt.Time_NE < earliest)
			{
				earliest = Self.Rt.Time_NE;
			}
		}
		if(CHECKBIT(rNER, Self.Rt.Retransmitables))
		{
			if(Act > Self.Rt.Time_NER)
			{
				// Transmit NER
				printf("Retransmitting an NER\n");
				addToQueue(newOutMessage(getPacketSize(Self.Rt.NER_ret_msg), Self.Rt.NER_ret_msg), 8, Self.OutboundQueue, 1);
				Self.Rt.NER_ret_amm += 1;
				Self.Rt.Time_NER += RETRANSMISSION_DELAY_NER;
			}
			else if(Self.Rt.Time_NER < earliest)
			{
				earliest = Self.Rt.Time_NER;
			}
		}

		pthread_mutex_unlock(&(Self.Rt.Lock));
		usleep((unsigned int)((earliest-Act)/1E3));
	}
	return NULL;
}


#include "time_table.h"

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

bool beginTBTransmission()
{
	unsigned long int Act;
	timespec Res;

	pthread_mutex_lock(&(Self.Rt.Lock));
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
	// A TB is already being retransmitted
	if(CHECKBIT(rTB, Self.Rt.Retransmitables))
	{
		// The TB was already transmitted at least once
		if(Self.Rt.TB_ret_amm)
		{
			// Ignore for now
			pthread_mutex_unlock(&(Self.Rt.Lock));
			return false;
		}// The TB hasn't yet been transmitted
		else
		{
			// Just create the TB with the new slave
			// Do not update the transmission time
			free(Self.Rt.TB_ret_msg);
			Self.Rt.TB_ret_msg = generateTB();
		}
	}
	else
	{
		Self.Rt.Time_TB = Act + TB_GENERATION_DELAY;
		Self.Rt.TB_ret_msg = generateTB();
		SETBIT(rTB, Self.Rt.Retransmitables);
	}
	pthread_mutex_unlock(&(Self.Rt.Lock));
	return true;
}

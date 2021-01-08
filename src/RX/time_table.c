#include "time_table.h"

void TB_RX(in_message* msg)
{
	void* buff = msg->buf;
	bool send_TA = false;
	bool retransmit_TB = false;
	byte* local_byte;
	int ip_amm;
	byte slot;
	byte PBID[2];
	byte senderIP[2];
	unsigned long int prev_table_size = Self.NewTimeTable->table_size;

	senderIP[0] = ((byte*)(buff))[1];
	senderIP[1] = ((byte*)(buff))[2];
	if((senderIP[0] == Self.IP[0] && senderIP[1] == Self.IP[1]) || Self.IsMaster)
	{
		clearInMessage(msg);
		return;
	}

	pthread_mutex_lock(&(Self.NewTimeTable->Lock));
	Self.NewTimeTable->local_slot = -1;
	unsigned long int validity_delay = (unsigned long int)(*((unsigned short*)(((byte*)buff+13))))*1E3;
	Self.NewTimeTable->sync = *((unsigned long int*)(((byte*)buff+5)));
	Self.NewTimeTable->timeslot_size = *(((byte*)buff+15))*1E6;
	ip_amm = *((short*)(((byte*)buff+16)));
	Self.NewTimeTable->table_size = ip_amm*Self.NewTimeTable->timeslot_size;
	printf("SYNC TIMESTAMP %lu %lu\n", Self.NewTimeTable->sync, validity_delay);
	Self.NewTimeTable->sync += validity_delay;

	timespec res;
	clock_gettime(CLOCK_REALTIME, &res);
	printf("%lu\n", res.tv_sec * (int64_t)1000000000UL + res.tv_nsec);
	for(int i = 0; i < ip_amm; i++)
	{
		if(((short*)(((byte*)buff+18)))[i] == ((short*)Self.IP)[0])
		{
			Self.NewTimeTable->local_slot = i;
			slot = i;
			break;
		}
	}
	if(Self.NewTimeTable->local_slot == -1)
	{
		dumpBin((char*)buff, getPacketSize(buff), "Did not receive timeslot from TB\n");
		// SET STATE TO OUTSIDE NETWORK
		clearInMessage(msg);
		return;
	}
	local_byte = ((byte*)buff)+18+ip_amm*2 + (slot/8);
	slot = slot - 8 * (slot/8);
	send_TA = (0x80 >> slot) & local_byte[0];
	for(int i = 0; i < Self.SubSlaves->L->Size; i++)
	{
		retransmit_TB |= getBitmapValue(getIPFromList(Self.SubSlaves, i), (byte*)buff+18+ip_amm*2, ip_amm, (byte*)buff+18);
	}

	if(send_TA)
	{
		TA_TX(Self.IP, Self.TB_PBID);
	}
	dumpBin((char*)buff, getPacketSize(buff), "Received TB, place = %d TA = %d rTB=%d %d\n", slot, send_TA, retransmit_TB, retransmit_TB && (((byte*)buff)[3] != Self.TB_PBID[0] || ((byte*)buff)[4] != Self.TB_PBID[1]));

	// Need to retransmit
	if(retransmit_TB &&
			// PBID is new
			((((byte*)buff)[3] != Self.TB_PBID[0] || ((byte*)buff)[4] != Self.TB_PBID[1]) ||
			// PBID is "old", but it's the first one and it's a new table
			(((byte*)buff)[3] == 0 && ((byte*)buff)[4] == 0 && prev_table_size != Self.NewTimeTable->table_size)))
	{
		Self.TB_PBID[0] = ((byte*)buff)[3];
		Self.TB_PBID[1] = ((byte*)buff)[4];
		((byte*)(buff))[1] = Self.IP[0];
		((byte*)(buff))[2] = Self.IP[1];
		printf("Retransmitting received TB\n");
		TB_TX(buff);
	}
	pthread_mutex_unlock(&(Self.NewTimeTable->Lock));
	clearInMessage(msg);
	Self.Status = Inside;
}


void TA_RX(in_message* msg)
{
	byte SenderIp[2];
	byte PBID[2];
	byte Originator_IP[2];

	SenderIp[0]=((byte*)msg->buf)[1];
	SenderIp[1]=((byte*)msg->buf)[2];
	
	Originator_IP[0]=((byte*)msg->buf)[3];
	Originator_IP[1]=((byte*)msg->buf)[4];

	PBID[0]=((byte *)msg->buf)[5];
	PBID[1]=((byte *)msg->buf)[6];
	void * TBmessage;
	TBmessage = Self.Rt.TB_ret_msg;
	if(SenderIp[0] == Self.IP[0] && SenderIp[1] == Self.IP[1])
	{
		clearInMessage(msg);
		return;
	}
	if(Self.IsMaster) // If Master, sets the corresponding Originator IPs' bit to 0 in the bitmap of the next TB retransmission
	{
		pthread_mutex_lock(&(Self.Rt.Lock));
        if(TBmessage == NULL)
		{
			pthread_mutex_unlock(&(Self.Rt.Lock));
			clearInMessage(msg);
			return;
		}
		short ip_amm;
		ip_amm = ((short*)(((byte*)TBmessage+16)))[0];

		dumpBin((char*)TBmessage, getPacketSize(TBmessage), "before");
		clearBitmapValue((short*)Originator_IP, (byte*)TBmessage+18+ip_amm*2, ip_amm, (byte*)TBmessage+18);
		dumpBin((char*)TBmessage, getPacketSize(TBmessage), "after");
		pthread_mutex_unlock(&(Self.Rt.Lock));
		for(int i = 0; i < ip_amm; i++)
		{
			if((byte*)((byte*)TBmessage+18+ip_amm*2)[i])
			{
				clearInMessage(msg);
				return;
			}
		}
		sendNetStats(STAT_TIMETABLE);
		stopRetransmission(rTB);
	}
	else
	{
		if(getSubSlave(Originator_IP) && !pbidSearchPair(Originator_IP, PBID, Self.PBID_IP_TA))
		{
			TA_TX(Originator_IP, PBID);
			pbidRemovePair(Originator_IP, Self.PBID_IP_TA);
			pbidInsertPair(Originator_IP, PBID, Self.PBID_IP_TA);
		}
		
	}
	clearInMessage(msg);
}

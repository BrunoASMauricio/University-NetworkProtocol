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

	pthread_mutex_lock(&(Self.TimeTable->Lock));
	Self.TB_PBID[0] = ((byte*)buff)[0];
	Self.TB_PBID[1] = ((byte*)buff)[1];
	Self.TimeTable->local_slot = -1;
	Self.TimeTable->table_size = ((short*)(((byte*)buff+16)))[0];
	ip_amm = Self.TimeTable->table_size;
	for(int i = 0; i < ip_amm; i++)
	{
		if(((short*)(((byte*)buff+18)))[i] == ((short*)Self.IP)[0])
		{
			Self.TimeTable->local_slot = i;
			slot = i;
			break;
		}
	}
	if(Self.TimeTable->local_slot == -1)
	{
		dumpBin((char*)buff, getPacketSize(buff), "Did not receive timeslot from TB\n");
		// SET STATE TO OUTSIDE NETWORK
		return;
	}
	Self.TimeTable->timeslot_size = (((byte*)buff+15))[0];
	local_byte = ((byte*)buff)+18+ip_amm*2 + (slot/8);
	slot = slot - 8 * (slot/8);
	send_TA = (0x80 >> slot) & local_byte[0];
	dumpBin((char*)buff, getPacketSize(buff), "Received TB, place = %d TA = %d\n", slot, send_TA);
	for(int i = 0; i < Self.SubSlaves->L->Size; i++)
	{
		retransmit_TB |= getBitmapValue(getIPFromList(Self.SubSlaves, i), (byte*)buff+18+ip_amm*2, ip_amm, (byte*)buff+18);
	}

	if(send_TA)
	{
		TA_TX(Self.IP, Self.TB_PBID);
	}

	if(retransmit_TB && ((byte*)buff)[0] != Self.TB_PBID[0] && ((byte*)buff)[1] == Self.TB_PBID[1])
	{
		TB_TX(buff);
	}
	pthread_mutex_unlock(&(Self.TimeTable->Lock));

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
	if(Self.IsMaster) // If Master, sets the corresponding Originator IPs' bit to 0 in the bitmap of the next TB retransmission
	{
		pthread_mutex_lock(&(Self.Rt.Lock));
        if(TBmessage == NULL)
		{
			pthread_mutex_unlock(&(Self.Rt.Lock));
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
				return;
			}
		}
		stopRetransmission(rTB);
	}
	else
	{
		if(getSubSlave(Originator_IP) && pbidSearchPair(Originator_IP, PBID, Self.PBID_IP_TA))
		{
			TA_TX(Originator_IP, PBID);
			pbidRemovePair(Originator_IP, Self.PBID_IP_TA);
			pbidInsertPair(Originator_IP, PBID, Self.PBID_IP_TA);
		}
		
	}
}

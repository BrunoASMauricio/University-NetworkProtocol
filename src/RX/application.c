#include "application.h"

void SD_RX(in_message* msg)
{
	int math = 15; //0000 1111

	int SampleNum=0;
		SampleNum = ((byte*)msg->buf)[6];

	byte NextHopIp[2];
	NextHopIp[0] = ((byte*)msg->buf)[3];
	NextHopIp[1] = ((byte*)msg->buf)[4];
	byte SourceIP[2];
	SourceIP[0] = ((byte*)msg->buf)[1];
	SourceIP[1] = ((byte*)msg->buf)[2];

	// Ignore all packets that don't have this node as the NextHop
	if(!(Self.IP[0] == NextHopIp[0] && Self.IP[1] == NextHopIp[1]))
	{
		clearInMessage(msg);
		return;
	}
	
	unsigned long int Act;
	timespec Res;
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

	routUpdateLastHeard(Self.Table, SourceIP);
	// Am I the next hop?
	byte sub_slave_IP[2];

	sub_slave_IP[0]= ((byte*)msg->buf)[1];
	sub_slave_IP[1]= ((byte*)msg->buf)[2];

	insertSubSlave(sub_slave_IP);
	insertIPList(Self.OutsidePending, sub_slave_IP);


	table_entry* am_i_sub_slave = routSearchByIp(Self.Table, sub_slave_IP);

	if(am_i_sub_slave == NULL)
	{
		printf("The received IP of the sender is not a SubSlave So Im adding\n");
		routInsertOrUpdateEntry(Self.Table, sub_slave_IP,UNREACHABLE, msg->PBE, 1, msg->received_time, getPacketSize(msg->buf));
	}
	else
	{
		routUpdateRollingLocalPBE(sub_slave_IP, msg->PBE, getPacketSize(msg->buf));
	}
	//caso seja master vai para a Q
	if(Self.IsMaster)
	{
		byte* DataToHW = (byte*)malloc(((byte*)(msg->buf))[6] + 2);
		DataToHW[0] = ((byte*)msg->buf)[1];
		DataToHW[1] = ((byte*)msg->buf)[2];					//dao source IP á HW

		//Aqui fica formado o pacote para HW 
		for(int i = 0; i < ((byte*)msg->buf)[6]; i++)
		{
			DataToHW[i+2] = ((byte*)msg->buf)[7+i];
		}

		dumpBin((char*)DataToHW, ((byte*)(msg->buf))[6] + 2, "ADDING TO INTERNAL QUEUE\n");
		addToQueue(DataToHW, ((byte*)(msg->buf))[6] + 2, Self.InternalQueue, 1);
	}
	else
	{
		//sends the sample nº to SD_TX
		out_message* out_sd = buildSDMessage(((byte*)msg->buf)+7, ((byte*)(msg->buf))[6], sub_slave_IP);
		if(!out_sd)
		{
			return;
		}
		addToQueue(out_sd, getPacketSize(out_sd->buf), Self.OutboundQueue, 1);
	}
	clearInMessage(msg);
}





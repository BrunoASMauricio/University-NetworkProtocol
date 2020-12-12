#include "application.h"

void SD_RX(in_message* msg)
{
	int math = 15; //0000 1111

	int SampleNum=0;
		SampleNum = ((byte*)msg->buf)[6];

	byte NextHopIp[2];
	NextHopIp[0] = ((byte*)msg->buf)[3];
	NextHopIp[1] = ((byte*)msg->buf)[4];
	
	//checks para verificação de erros
	if((((byte*)msg->buf)[0]&math) != 1)
	{
		fatalErr("Error: how did you even get here, a not SD packet is inside SD, message[0]) %d", ((byte*)msg->buf)[0]&math);
	}
	//checks para verificação de erros
	if(((byte*)msg->buf)[2] == 0)
	{
		fatalErr("Error: TTL was 0 when should not");
	}

	//add [IP | SAMPEN | SAMPLE1 | SAMPLE2 ...]
	
	unsigned long int Act;
	timespec Res;
	// Am I the next hop?
	if(Self.IP[0] == NextHopIp[0] && Self.IP[1] == NextHopIp[1])
	{
		byte sub_slave_IP[2];

		sub_slave_IP[0]= ((byte*)msg->buf)[1];
        sub_slave_IP[1]= ((byte*)msg->buf)[2];

		insertSubSlave(sub_slave_IP);
        insertIPList(Self.OutsidePending, sub_slave_IP);

        clock_gettime(CLOCK_REALTIME, &Res);
        Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

        table_entry* am_i_sub_slave = routSearchByIp(Self.Table, sub_slave_IP);

        if(am_i_sub_slave == NULL)
        {
            printf("The received IP of the sender is not a SubSlave So Im adding");
            routInsertOrUpdateEntry(Self.Table, sub_slave_IP,UNREACHABLE, msg->PBE, 1, msg->received_time);
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
			SD_TX(msg->buf+7, ((byte*)(msg->buf))[6]);
		}

	}
	clearInMessage(msg);
}





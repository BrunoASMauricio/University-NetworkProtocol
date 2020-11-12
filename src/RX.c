#include "RX.h"


void*
WF_listener(void* dummy)
{
	printf("WF Listener on\n");
	while(1){sleep(1);}

}

in_message*
getMessage()
{
	return NULL;
}

void*
WS_listener(void* dummy)
{
	printf("WS Listener on\n");
	while(1)
    {
        sleep(1);
    }
}

void SD_RX(in_message* msg)
{
	int math = 15; //0000 1111

	int SampleNum=0;
		SampleNum = ((byte*)msg->buf)[6];
	
	if(((byte*)msg->buf)[0]&math != 1)
	{
	fatalErr("Error: how did you even get here, a not SD packet is inside SD, message[0]) %d", ((byte*)msg->buf)[0]&math);
	}

	if(((byte*)msg->buf)[2] == 0)
	{
		fatalErr("Error: TTL was 0 when should not");
	}

	//caso seja master vai para a Q do HW
	if(Self.IsMaster == true)
	{
		//add para a queue da interface de HW do rodrigo [IP | SAMPLE]  e depois mudar para isto[IP | SAMPE | TIMESTAMP]
		//ainda nao me disseram onde vinha o a timestamp por isso vai ficar aqui parado á espera.

		byte DataToHW[((byte*)msg->buf)[6] + 2];
		DataToHW[0] = ((byte*)msg->buf)[1];
		DataToHW[1]=((byte*)msg->buf)[4];//dao source IP á HW

		int SizeOfPacket;
		SizeOfPacket = getPacketSize(DataToHW);

		addToQueue( newInMessage(SizeOfPacket + SampleNum , DataToHW ,res), SizeOfPacket , Self.OutboundQueue, 1);

		for  (int i = 0; i < ((byte*)msg->buf)[7]; i++)
		{
			DataToHW[i+2] = ((byte*)msg->buf)[7+i];
		}
	

	}

	//Caso seja slave faz foward para o proximo node passando as ser proxy
	if(Self.IsMaster == false)
	{
		/*Add the Source IP to the Sub-Slaves, updating LastHeard
		(Note @AntonioMendes think this is already done in SD_TX if it enters there not sure)*/

		//sends the sample nº to SD_TX 
		SD_TX(SampleNum);


	}
}

void PB_RX(in_message* msg)
{
	return;
}

void PR_RX(in_message* msg)
{
	return;
}

void PC_RX(in_message* msg)
{
	return;
}

void TA_RX(in_message* msg)
{
	return;
}

void TB_RX(in_message* msg)
{
	return;
}

void NE_RX(in_message* msg)
{
	return;
}

void NEP_RX(in_message* msg)
{
	return;
}

void NER_RX(in_message* msg)
{
	return;
}

void NEA_RX(in_message* msg)
{
	return;
}



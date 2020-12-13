#include "routing.h"

void PB_RX(in_message* msg)
{
	void* buff;
	byte SenderIp[2];
	byte PBID[2];
	SenderIp[0]=((byte*)msg->buf)[1];
	SenderIp[1]=((byte*)msg->buf)[2];
	PBID[0]=((byte *)msg->buf)[3];
	PBID[1]=((byte *)msg->buf)[4];
	unsigned short distance	=(((byte *)msg->buf)[5]<< 8) + ((byte *)msg->buf)[6];


	timespec Res;
	unsigned long int Act;


	if(Self.Status == Outside)
	{ //if the node is an outside slave 

		if(distance!= (unsigned short)65535)
		{
			printf("As an outside slave, received a PB with %u distance to Master\n", distance);
			void* NEMessage = buildNEMessage(Self.IP, SenderIp);
			NE_TX(NEMessage);
			startRetransmission(rNE, NEMessage);
			clearInMessage(msg);
			return;
		}	
	}
	else
	{
		if(!pbidSearchPair(SenderIp, PBID, Self.RoutingPBIDTable))
		{
			pbidInsertPair(SenderIp, PBID, Self.RoutingPBIDTable); //stores pair in PBID table

			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

			routInsertOrUpdateEntry(Self.Table, SenderIp, distance, 0,0,Act); //stores distance when receiveing PB so later when it receives PC can update
			buff = buildPRMessage(SenderIp, PBID, msg->PBE);
			PR_TX(buff);
			startRetransmission(rPR, buff);
		}
	}
	clearInMessage(msg);
}





void PR_RX(in_message* msg)
{
	byte SenderIp[2];
	byte OriginatorIp[2];
	byte PBID[2];
	float PBEofSentPB;
	SenderIp[0]=((byte*)msg->buf)[1];
	SenderIp[1]=((byte*)msg->buf)[2];
	OriginatorIp[0]=((byte*)msg->buf)[3];
	OriginatorIp[1]=((byte*)msg->buf)[4];
	PBID[0]=((byte *)msg->buf)[5];
	PBID[1]=((byte *)msg->buf)[6];
	unsigned short distance =(((byte *)msg->buf)[7]<<8) + ((byte *)msg->buf)[8];
	PBEofSentPB = *((float*)(&(((byte *)msg->buf)[9])));

	
	timespec Res;
	unsigned long int Act;
	table_entry* SenderEntry;
	unsigned short Distance;

	//if new pair, check everything, if  not check if the extrapolated distance it's better

	if(OriginatorIp[0]== Self.IP[0] && OriginatorIp[1]== Self.IP[1]) //the node is receiving a PR from a PB it generated
	{
		if(pbidSearchPair(SenderIp,PBID,Self.RoutingPBIDTable)==0)
		{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			//somehow update in routTable using existance distance, SNRofSentPB (remote snr) and msg->snr (local snr)
			Distance=updateDistance(PBEofSentPB, 0,0);
			routInsertOrUpdateEntry(Self.Table, SenderIp, Distance, msg->PBE, PBEofSentPB,Act);
			PC_TX(SenderIp,PBID,msg->PBE);
			Self.PBID++;//this only makes sense to update if it hasn't received that pair

		}

		else
		{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			//somehow checks if the extrapolated distance it's better than one we have 
			Distance=updateDistance(PBEofSentPB, 0,0);
			SenderEntry = routSearchByIp(Self.Table, SenderIp);
			if(SenderEntry->Distance > Distance)
			{
				routInsertOrUpdateEntry(Self.Table, SenderIp, distance, msg->PBE, PBEofSentPB,Act);
				PC_TX(SenderIp,PBID,msg->PBE); //warning that there's 
			}
		}
		
	}
	clearInMessage(msg);
}
void PC_RX(in_message* msg)
{

	byte SenderIP[2];
	byte ReachedIP[2];
	byte PBEofSentPR;
	byte PBID[2];
	SenderIP[0]=((byte*)msg->buf)[1];
	SenderIP[1]=((byte*)msg->buf)[2];
	ReachedIP[0]=((byte*)msg->buf)[3];
	ReachedIP[1]=((byte*)msg->buf)[4];
	PBID[0]=((byte *)msg->buf)[5];
	PBID[1]=((byte *)msg->buf)[6];
	PBEofSentPR = *((float*)(&(((byte *)msg->buf)[7])));


	timespec Res;
	unsigned long int Act;
	table_entry* SenderEntry;
	float Distance;

	if(ReachedIP[0]== Self.IP[0] && ReachedIP[1]== Self.IP[1])
	{

		if(pbidSearchPair(SenderIP,PBID,Self.RoutingPBIDTable)==0)
		{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			//somehow update in routTable using existance distance, SNRofSentPR (remote snr) and msg->snr (local snr)
			Distance=updateDistance(PBEofSentPR,0,0); 
			routInsertOrUpdateEntry(Self.Table, SenderIP, Distance, msg->PBE, PBEofSentPR,Act);
			stopRetransmission(rPR); 
			startRetransmission(rPB, createPB());
		}

		else
		{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			//somehow checks if the extrapolated distance it's better than one we have 
			Distance=updateDistance(PBEofSentPR,0,0); 
			SenderEntry = routSearchByIp(Self.Table, SenderIP);
			if(SenderEntry->Distance > Distance)
			{
				routInsertOrUpdateEntry(Self.Table, SenderIP, Distance, msg->PBE, PBEofSentPR,Act);
			}
		}
		
	}	
	
	clearInMessage(msg);
}


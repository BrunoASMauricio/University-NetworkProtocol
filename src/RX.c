#include "RX.h"
#include "data.h"
#include "protocol.h"


void*
WF_listener(void* dummy)
{
	char buff[MAXIMUM_PACKET_SIZE*2];		// *2 to prevent overflow on buff+PrevBytes
	int PacketSize;
	int ReadBytes = 0;
	int PrevBytes = 0;
	unsigned int received_messages = 0;
	in_message* message;
	timespec res;

	printf("WF Listener on port %u\n", Meta.WF_RX->port);

	// This is needed to kickstart the connection
	// Everywhere I looked, client always spoke first
	// Always found "in UDP it doesn't matter who talks first"
	// But if client doesn't send this ping, it doesn't work
	sleep(1);
	sendToSocket(Meta.WF_RX, &PrevBytes, 2);

	PrevBytes = 0;
	while(1)
	{
		while((ReadBytes = getFromSocket(Meta.WF_RX, buff+PrevBytes)) == -1)
		{
			continue;
		}
		printf("\t\t-------Node got packet (%d bytes) total of %d!!-------\n", ReadBytes, ++received_messages);

		if(((byte*)buff)[0] & 0x0f == TB && ReadBytes < 18)
		{
			PrevBytes = ReadBytes;
			printf("Got truncated TB\n");
			continue;
		}

		clock_gettime(CLOCK_REALTIME, &res);

		PacketSize = getPacketSize(buff);

		if(PacketSize == -1)
		{
			dumpBin(buff, ReadBytes, "Packet size returned -1, dumping buffer\n");
			continue;
		}

		if(PacketSize > ReadBytes + PrevBytes)
		{
			dumpBin(buff, ReadBytes, "Packet size (%d) is more than what was received (%d).\n", PacketSize, ReadBytes + PrevBytes);
			PrevBytes = ReadBytes;
			continue;
		}

		addToQueue(newInMessage(PacketSize, buff, res), 8, Self.InboundQueue, 1);

		// We received more than one packet
		if(PacketSize < ReadBytes + PrevBytes)
		{
			printf("\t\tMore than one packet (%d, %d)\n", PacketSize, ReadBytes+PrevBytes);
			// Copy the last of the read bytes, to the beggining of the buffer
			for(int i = 0; PacketSize + i < ReadBytes + PrevBytes; i++)
			{
				buff[i] = buff[PacketSize+i];
			}
			PrevBytes = PacketSize - (PrevBytes + ReadBytes);
		}
		else
		{
			PrevBytes = 0;
		}
	}
}

in_message*
getMessage()
{
	return NULL;
}


void*
WS_listener(void* dummy)
{
        printf("WS Listener ON\n");

        int aux=1, nBytes,nBytesAux=0;
        byte *buff;
        byte* TotalSample = (byte*)malloc(sizeof(byte)*TAMTOTALSAMPLE); 
        byte* TotalSample_aux = (byte*)malloc(sizeof(byte)*TAMTOTALSAMPLE);

        socket_s* sockfd = newSocket(PORTWS);
        startSocket_ws(sockfd);

        if (setsockopt(sockfd->s,SOL_SOCKET,SO_REUSEADDR,&aux,sizeof(int)) == -1)
        {
            perror("Setsockopt");
            exit(1);
        }

        if (bind(sockfd->s, (const struct sockaddr*) &(sockfd->sockaddr), sizeof(sockfd->sockaddr)) < 0) 
        { 
            perror("bind failed"); 
            exit(EXIT_FAILURE); 
        }

		buff = (byte *)malloc(sizeof(byte)*MAX_TRANS_SIZE);
        if (Self.IsMaster == 1)
        {
            
			while(1)
            {
                nBytes = getFromSocket(sockfd,buff);
				
				if (nBytes < DATAPAYLOAD)
				{
					memcpy(TotalSample+TAMIP+nBytesAux, buff, nBytes);
					nBytesAux += nBytes;
					if (nBytesAux >= DATAPAYLOAD){
						memcpy(TotalSample, Self.IP , TAMIP);
						addToQueue((void*)TotalSample, sizeof(byte)*TAMTOTALSAMPLE, Self.InternalQueue, 1);
						TotalSample = (byte*)malloc(sizeof(byte)*TAMTOTALSAMPLE);
						nBytesAux = 0; 
					}

				} else
				{
					memcpy(TotalSample, Self.IP , TAMIP);
					memcpy(TotalSample+TAMIP, buff, DATAPAYLOAD);
					addToQueue((void*)TotalSample, sizeof(byte)*TAMTOTALSAMPLE, Self.InternalQueue, 1);
					TotalSample = (byte*)malloc(sizeof(byte)*TAMTOTALSAMPLE); 
				}
					
             }
			 
        } 
        else 
        {
            
			while(1)
            {
                nBytes = getFromSocket(sockfd,buff);
				
				if (nBytes < DATAPAYLOAD)
				{
					memcpy(TotalSample+nBytesAux, buff, nBytes);
					nBytesAux += nBytes;
					if (nBytesAux >= DATAPAYLOAD){
						addToQueue((void*)TotalSample, sizeof(byte)*DATAPAYLOAD, Self.InternalQueue, 0);
						TotalSample = (byte*)malloc(sizeof(byte)*DATAPAYLOAD);
						nBytesAux = 0;

						// chamar função do Schumacher   
						// pega nas data TotalSample (por exemplo 10 samples  == MAXTotalSample) da queue interna  
						// e nete num pacote SD para a Self.Outboudqueue.
						// Schumacher(pointer para queue das samples, numero de samples, timestamp); 
					}

				} else
				{
					memcpy(TotalSample, buff, DATAPAYLOAD);
					addToQueue((void*)TotalSample, sizeof(byte)*DATAPAYLOAD, Self.InternalQueue, 0);
					TotalSample = (byte*)malloc(sizeof(byte)*DATAPAYLOAD);

					// chamar função do Schumacher   
					// pega nas data TotalSample (por exemplo 10 samples  == MAXTotalSample) da queue interna  
					// e nete num pacote SD para a Self.Outboudqueue.
					// Schumacher(pointer para queue das samples, numero de samples, timestamp);
					
				}                
				
				
                
            }
			
        }
		free(buff);    
        close(sockfd->s);
		return NULL;
}

void SD_RX(in_message* msg)
{
	return;
}

void PB_RX(in_message* msg)
{
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
			NE_TX(SenderIp);//sends a NE
			//startRetransmission(rNE);
			return; 
		}	
	}
	else
	{
		/*if(pibdSearchPair(SenderIp,PBID,Self.RoutingPBIDTable)==0)
		{
			pbidInsertPair(SenderIP,PBID,Self.RoutingPBIDTable); //stores pair in PBID table

			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

			routInsertOrUpdateEntry(Self.Table, SenderIp, distance, 0,0,Act); //stores distance when receiveing PB so later when it receives PC can update
			PR_TX(SenderIp, PBID, msg->SNR);
			startRetransmission(rPR);
		}*/
	}

	delInMessage(msg);
	return;
}

void PR_RX(in_message* msg)
{
	byte SenderIp[2];
	byte OriginatorIp[2];
	byte PBID[2];
	byte SNRofSentPB;
	SenderIp[0]=((byte*)msg->buf)[1];
	SenderIp[1]=((byte*)msg->buf)[2];
	OriginatorIp[0]=((byte*)msg->buf)[3];
	OriginatorIp[1]=((byte*)msg->buf)[4];
	PBID[0]=((byte *)msg->buf)[5];
	PBID[1]=((byte *)msg->buf)[6];
	unsigned short distance =(((byte *)msg->buf)[7]<<8) + ((byte *)msg->buf)[8];
	SNRofSentPB=((byte *)msg->buf)[9];
	
	timespec Res;
	unsigned long int Act;
	table_entry* SenderEntry;

	//if new pair, check everything, if  not check if the extrapolated distance it's better

	if(OriginatorIp[0]== Self.IP[0] && OriginatorIp[1]== Self.IP[1]) //the node is receiving a PR from a PB it generated
	{
		//if(pibdSearchPair(SenderIp,PBID,Self.RoutingPBIDTable)==0)
		//{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			//somehow update in routTable using existance distance, SNRofSentPB (remote snr) and msg->snr (local snr)
			//distance=updateDistance(distance, SNRofSentPB, msg->snr) or something like that 
			//routInsertOrUpdateEntry(Self.Table, SenderIp, distance, msg->SNR, SNRofSentPB,Act);
			PC_TX(SenderIp,PBID,msg->SNR);
			Self.PBID++;//this only makes sense to update if it hasn't received that pair

		//}

		//else
		//{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			//somehow checks if the extrapolated distance it's better than one we have 
			SenderEntry = routSearchByIp(Self.Table, SenderIp);
			/*if(SenderEntry->Distance > extrapolated distance)
			{
				routInsertOrUpdateEntry(Self.Table, SenderIp, distance, msg->SNR, SNRofSentPB,Act);
				PC_TX(SenderIp,PBID,msg->SNR); //warning that there's 
			}*/
		//}
		
	}
	delInMessage(msg);
	return;
}

void PC_RX(in_message* msg)
{

	byte SenderIP[2];
	byte ReachedIP[2];
	byte SNRofSentPR;
	SenderIP[0]=((byte*)msg->buf)[1];
	SenderIP[1]=((byte*)msg->buf)[2];
	ReachedIP[0]=((byte*)msg->buf)[3];
	ReachedIP[1]=((byte*)msg->buf)[4];
	SNRofSentPR =((byte*)msg->buf)[7];

	timespec Res;
	unsigned long int Act;

	if(ReachedIP[0]== Self.IP[0] && ReachedIP[1]== Self.IP[1])
	{

		/*if(pibdSearchPair(SenderIp,PBID,Self.RoutingPBIDTable)==0)
		{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			somehow update in routTable using existance distance, SNRofSentPR (remote snr) and msg->snr (local snr)
			//distance=updateDistance(distance, SNRofSentPR, msg->snr) or something like that 
			routInsertOrUpdateEntry(Self.Table, SenderIP, distance, msg->SNR, 0); gotta check the distance with Bruno M
			stopRetransmission(nPR);
		}

		else
		{
			clock_gettime(CLOCK_REALTIME, &Res);
			Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
			//somehow checks if the extrapolated distance it's better than one we have 
			SenderEntry = routSearchByIp(Self.Table, SenderIp);
			if(SenderEntry->Distance > ExtrapolatedDistance)
			{
				routInsertOrUpdateEntry(Self.Table, SenderIp, distance, msg->SNR, SNRofSentPB,Act);
			}
		}*/
	}	
	
	delInMessage(msg);
	return;
}

void TA_RX(in_message* msg)
{
	return;
}

void TB_RX(in_message* msg)
{
	void* buff = msg->buf;
	bool send_TA = false;
	bool retransmit_TB = false;
	byte* local_byte;
	int ip_amm;
	byte slot;
	// New PBID? Accept new timeslot
	
	pthread_mutex_lock(&(Self.TimeTable->Lock));
	if(((byte*)buff)[0] != Self.TB_PBID[0] && ((byte*)buff)[1] == Self.TB_PBID[1])
	{
		Self.TimeTable->local_slot = -1;
		Self.TimeTable->table_size = ((short*)(((byte*)buff+16)))[0];
		ip_amm = Self.TimeTable->table_size;
		for(int i = 0; i < ip_amm; i++)
		{
			if(((short*)(((byte*)buff+18)))[i] == ((short*)Self.IP)[0])
			{
				Self.TimeTable->local_slot = i;
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
		// TA_TX()
	}

	if(retransmit_TB)
	{
		// TB_TX()
	}
	
	pthread_mutex_unlock(&(Self.TimeTable->Lock));
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
	byte* Packet = (byte*)msg->buf;
    //NOTE(GoncaloXavier): This assumes msg.buff 
    //has a normal NER packet format!
    if(Self.IP[0] == Packet[1] && Self.IP[1] == Packet[2])
    {
        //Add the Outsider IP to the Sub-Slaves, updating LastHeard
        insertSubSlave(&Packet[3]);

        unsigned long int Act;
        timespec Res;
        clock_gettime(CLOCK_REALTIME, &Res);
        Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

        table_entry* Outsider = routSearchByIp(Self.Table, &Packet[3]);
        //TODO: This might make sense to be done using update function
        Outsider->LastHeard = Act;

        if(Self.IsMaster)
        {
            //NOTE(GoncaloXavier): This assumes generateTB() generates deadline
            //TODO(GoncaloXavier): Check if it does...
            generateTB();
            
            //Sends NEA Message back
            //Send Outsiders IP and PBID to NEA
            pbid PBID = getNewPBID();
            NEA_TX(&Packet[3], PBID);
        }
        else
        {
            //Transmit the packet up the network
            //Send Outsiders' IP NER_TX
            out_message* NERMessage = NER_TX(&Packet[3]);
            startRetransmission(rNER, NERMessage->buf);
        }
    } 
    
    //Discard packet
    delInMessage(msg);
	return;

}

void NEA_RX(in_message* msg)
{
	return;
}

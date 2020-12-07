#include "RX.h"
#include "data.h"
#include "routing_table.h"

void
WF_listener()
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
		printf("Got %d bytes from socket\n", ReadBytes);
		if(((byte*)buff)[0] & 0x0f == TB && ReadBytes < 18)
		{
			PrevBytes = ReadBytes;
			printf("Got truncated TB\n");
			continue;
		}

		clock_gettime(CLOCK_REALTIME, &res);

		PacketSize = getPacketSize(buff) + 2; // Also get SNR for the message

		if(PacketSize == -1)
		{
			dumpBin(buff, ReadBytes+PrevBytes, "Packet size returned -1, dumping buffer\n");
			continue;
		}

		if(PacketSize > ReadBytes + PrevBytes)
		{
			dumpBin(buff, ReadBytes, "Packet size (%d) is more than what was received (%d).\n", PacketSize, ReadBytes + PrevBytes);
			PrevBytes = ReadBytes;
			continue;
		}
		printf("\t\t-------Node got message (%d/%d bytes) total of %d!!-------\n", ReadBytes, ReadBytes+PrevBytes, ++received_messages);


		message = newInMessage(PacketSize-2, buff, res);
		message->SNR = ((unsigned short*)(buff + PacketSize-2))[0];
		printf("Received full correct message! Received SNR: %u\n", message->SNR);
		printMessage(buff, PacketSize);
		// Directly handle message
		handler(message);

		//addToQueue(newInMessage(PacketSize, buff, res), 8, Self.InboundQueue, 1);

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


void*
WS_listener(void* dummy)
{
	printf("WS Listener ON\n");

	int aux=1, nBytes,nBytesAux=0;
	byte *buff;
	byte* TotalSample = (byte*)malloc(sizeof(byte)*TAMTOTALSAMPLE);
	byte* TotalSample_aux = (byte*)malloc(sizeof(byte)*TAMTOTALSAMPLE);

	socket_s* sockfd = newSocket(Meta.WS_port);
	startSocket_ws(sockfd);
	sleep(1);
	sendToSocket(sockfd, &nBytes, 2);


	if (setsockopt(sockfd->s,SOL_SOCKET,SO_REUSEADDR,&aux,sizeof(int)) == -1)
	{
		perror("Setsockopt");
		exit(1);
	}
	/*
	if (bind(sockfd->s, (const struct sockaddr*) &(sockfd->sockaddr), sizeof(sockfd->sockaddr)) < 0) 
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	*/
	buff = (byte *)malloc(sizeof(byte)*MAX_TRANS_SIZE);
	if (Self.IsMaster == 1)
	{
		while(1)
		{
			nBytes = getFromSocket(sockfd,buff);
			if(nBytes > 0)
			{
				memcpy(TotalSample, Self.IP , TAMIP);
				memcpy(TotalSample+TAMIP, buff, nBytes);
				if(!Self.IsMaster)
				{
					printf("RECEIVED MESSAGE FROM WS %d\n", nBytes);
				}
				//dumpBin((char*)TotalSample, nBytes+TAMIP, "Received from WS (%d+2 bytes): ", nBytes);
				addToQueue((void*)TotalSample, nBytes+TAMIP, Self.InternalQueue, 1);
				//TotalSample = (byte*)malloc(sizeof(byte)*TAMTOTALSAMPLE);
			}
		}
		/*
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
		 */
	}
	else
	{
		while(1)
		{
			nBytes = getFromSocket(sockfd,buff);
			if(nBytes > 0)
			{
				printf("RECEIVED MESSAGE FROM WS %u\n");
				//memcpy(TotalSample, buff, nBytes);
				//addToQueue((void*)TotalSample, sizeof(byte)*DATAPAYLOAD, Self.InternalQueue, 0);
				SD_TX(buff, nBytes);

				// chamar função do Schumacher
				// pega nas data TotalSample (por exemplo 10 samples  == MAXTotalSample) da queue interna
				// e nete num pacote SD para a Self.Outboudqueue.
				// Schumacher(pointer para queue das samples, numero de samples,
			}
			/*
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
			*/
		}
	}
	free(buff);
	close(sockfd->s);
	return NULL;
}

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
	
	byte DataToHW[((byte*)(msg->buf))[6] + 2];
	DataToHW[0] = ((byte*)msg->buf)[1];
	DataToHW[1] = ((byte*)msg->buf)[2];					//dao source IP á HW

	int SizeOfPacket;
	unsigned long int Act;
	timespec Res;
	SizeOfPacket = getPacketSize(DataToHW);
		
	//Aqui fica formado o pacote para HW 
	for(int i = 0; i < ((byte*)msg->buf)[6]; i++)
	{
		DataToHW[i+2] = ((byte*)msg->buf)[7+i];
	}

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
            routInsertOrUpdateEntry(Self.Table, sub_slave_IP,UNREACHABLE, msg->SNR, 1, msg->received_time);
        }

	}
	//caso seja master vai para a Q
	if(Self.IsMaster)
	{
		dumpBin((char*)DataToHW, ((byte*)(msg->buf))[6] + 2, "ADDING TO INTERNAL QUEUE\n");
		addToQueue(DataToHW, ((byte*)(msg->buf))[6] + 2, Self.InternalQueue, 1);
	}
	else
	{
	//sends the sample nº to SD_TX 
		//SD_TX(SampleNum);
	}
}

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
			buff = buildPRMessage(SenderIp, PBID, msg->SNR);
			PR_TX(buff);
			startRetransmission(rPR, buff);
		}
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

void NE_RX(in_message* msg)
{
	void* message;
    if(msg->buf == NULL)
    {
        printfErr("msg passed to NE_RX does not have NE packet format!\n");
        return;
    }
    
    byte* Packet = (byte*)msg->buf;
    
    if(Self.IP[0] == Packet[3] && Self.IP[1] == Packet[4])
    {
        //Add the Outsider IP to the Outside-Slaves, updating LastHeard
        byte SenderIP[2];
        SenderIP[0] = Packet[1];
        SenderIP[1] = Packet[2];
        insertSubSlave(SenderIP);  
        insertOutsideSlave(SenderIP);  

        unsigned long int Act;
        timespec Res;
        clock_gettime(CLOCK_REALTIME, &Res);
        Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
        
        //NOTE(GoncaloX): This is the 1st contact of an new node, as such
        //it shoul be added to the routTable.
        table_entry* Outsider = routInsertOrUpdateEntry(Self.Table, SenderIP, UNREACHABLE, msg->SNR, 1, msg->received_time);
        Outsider->LastHeard = Act;

        // se é o master que recebe NE, gera TimeBroadcast e gera NEP 
        // (NEP é sempre resposta de NE)
        if(Self.IsMaster)     
        {   
			beginTBTransmission();
            NEP_TX(SenderIP);
        }
        else
        {
            //NOTE(GoncaloX): Maybe this should also happen if node is Master?
            insertOutsideSlave(SenderIP);
            NEP_TX(SenderIP);

            // se é um node que não o master, transmite NER
			message = buildNERMessage(Self.Table->begin->Neigh_IP, SenderIP);
			NER_TX(message);
            startRetransmission(rNER, message);
        }
    }
}

void NEP_RX(in_message* msg)
{
    if(msg->buf == NULL)
    {
        printfErr("msg passed to NEP_RX does not have NEP packet format!\n");
        return;
    }
    
    byte* Packet = (byte*)msg->buf;
    if(Packet[3] == Self.IP[0] && Packet[4] == Self.IP[1])
    {
        // Cancel retransmission of NE 
        stopRetransmission(rNE);
        // Communication was established with possible proxy
        Self.Status = Waiting;
    }
    delInMessage(msg);
	return;
}

void NER_RX(in_message* msg)
{
	void* message;
	byte* Packet = (byte*)msg->buf;
    if(msg->buf == NULL)
    {
        printfErr("msg passed to NER_RX does not have NER packet format!\n");
        return;
    }
    
    if(Self.IP[0] == Packet[1] && Self.IP[1] == Packet[2])
    {
        //Add the Outsider IP to the Sub-Slaves, updating LastHeard
        //Add Outsider IP to Pending list
        //Since we delete msg at the end of this
        //function, this: insertSubSlave(&Packet[3]);
        //doesn't work, BIG SAD :( -> memcpy or manual assigned instead
        
        byte SubSlaveIP[2];
        SubSlaveIP[0]= Packet[3];
        SubSlaveIP[1]= Packet[4];
        insertSubSlave(SubSlaveIP);
        insertIPList(Self.OutsidePending, SubSlaveIP);

        unsigned long int Act;
        timespec Res;
        clock_gettime(CLOCK_REALTIME, &Res);
        Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

        table_entry* Outsider = routSearchByIp(Self.Table, SubSlaveIP);
        if(Outsider == NULL)
        {
            printf("IP received in NER not present in rouTable!\n");
            dumpBin((char*)(SubSlaveIP), 2, "Adding new entry with IP:");
            //TODO: Update Last Heard while adding entry
            routInsertOrUpdateEntry(Self.Table, SubSlaveIP,UNREACHABLE, msg->SNR, 1, msg->received_time);
        }

        if(Self.IsMaster)
        {
            //This assumes generateTB() generates deadline
            //TODO: Check if it does...
			beginTBTransmission();
            
            //Sends NEA Message back
            //Send Outsiders IP and PBID to NEA
            pbid PBID = getNewPBID();
            NEA_TX(&Packet[3], PBID);
        }
        else
        {
            //Transmit the packet up the network
            //Send Outsiders' IP NER_TX
            message = buildNERMessage(Self.Table->begin->Neigh_IP, &Packet[3]);
			NER_TX(message);
            //startRetransmission(rNER, message);
        }
    } 
    
    //Discard packet
    delInMessage(msg);
	return;

}

void NEA_RX(in_message* msg)
{
    if(msg->buf == NULL)
    {
        printfErr("msg passed to NEA_RX does not have NEA packet format!\n");
        return;
    }
    
    byte* Packet = (byte*)msg->buf;
    
    byte OutsiderIP[2];
    OutsiderIP[0] = Packet[1];
    OutsiderIP[1] = Packet[2];
    pbid PBID;
    PBID = (0<<4) + Packet[3];
    PBID = Packet[4];
    
    // se existir outside slave e for o outsider IP, quer dizer que chegamos ao proxy
    if(getOutsideSlave(OutsiderIP)) 
    {
        stopRetransmission(rNER);
    }
    else if(getIPFromList(Self.OutsidePending, OutsiderIP) == 0) 
    {
        return;
    }
    else    // se existir na lista de IPs, retiramos para cada node o outsider IP e retransmitimos até chegar ao Proxy
    {
        removeIPList(Self.OutsidePending, OutsiderIP);
        NEA_TX(OutsiderIP, PBID);
    }

	delInMessage(msg);
	return;
}

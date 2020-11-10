#include "RX.h"


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

	fprintf(stdout, "WF Listener on port %u\n", Meta.WF_RX->port);

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
		fprintf(stdout, "\t\t-------Node got packet (%d bytes) total of %d!!-------\n", ReadBytes, ++received_messages);
		PrevBytes = 0;

		if(clock_gettime(CLOCK_REALTIME, &res) == -1)
		{
			printfErr("Could not clock_gettime\n");
			// Ignore received messages here??? or handle unavailable timestamp elsewhere
			continue;
		}

		PacketSize = getPacketSize(buff);

		if(PacketSize == -1)
		{
			dumpBin(buff, ReadBytes, "Packet size returned -1, dumping buffer\n");
			continue;
		}

		if(PacketSize > ReadBytes)
		{
			// Undefined behaviour, just ignore for now
			// Eventually we could try to receive more and "complete the packet"?
			// This could fail so a "timer" would be needed. Too complex?
			dumpBin(buff, ReadBytes, "Packet size (%d) is more than what was received (%d).\n", PacketSize, ReadBytes);
			continue;
		}

		addToQueue(newInMessage(PacketSize, buff, res), 8, Self.InboundQueue, 1);

		// We received more than one packet
		if(PacketSize < ReadBytes)
		{
			// Copy the last of the read bytes, to the beggining of the buffer
			PrevBytes = PacketSize;
			for(int i = 0; PacketSize+i < ReadBytes; PacketSize++, i++)
			{
				buff[i] = buff[PacketSize+i];
			}
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
	return;
}

void NEA_RX(in_message* msg)
{
	return;
}



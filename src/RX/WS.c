#include "WS.h"



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
	//sleep(1);
	//sendToSocket(sockfd, &nBytes, 2);
	struct sockaddr_in addr;
	inet_aton("127.0.0.1", &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Meta.WS_port);
	bind(sockfd->s,(struct sockaddr*)&addr, sizeof(addr));


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


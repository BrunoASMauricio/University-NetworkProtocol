#include "HW.h"
	
void*
HW_dispatcher(void*dummy)
{
     printf("HW Dispatcher on\n");
    
    int PacketSize=0;  
    byte *Popped = (byte*)malloc(sizeof(byte)*(TOTALLENGTH));  


    socket_s* sockfd = newSocket(Meta.HW_port);
    startSocket(sockfd);

	if(!Self.IsMaster)
	{
		// Slaves don't require HW dispatcher
		return NULL;
	}
    while (1)
    {
        //  Sending========>    IP     DataPayload      // 
       //                       2bytes 3bytes          //  possible changes of these values // 
        
		Popped = (byte*) popFromQueue(&PacketSize,Self.InternalQueue);
		
		if (Popped == NULL) PacketSize = 0;

		if (PacketSize > 0)
		{
			dumpBin((char*)Popped, PacketSize, "Sending to HW (%d bytes): ", PacketSize);
			if(Self.IsMaster)
			{
				//printf("SENT MESSAGE TO HW %d\n", PacketSize);
			}
			sendToSocket(sockfd, Popped ,sizeof(byte)*PacketSize);
		}

		PacketSize=0;
		usleep(HW_DISPATCHER_SLEEP);
    }
    free(Popped); 
    close(sockfd->s); 
}



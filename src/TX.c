#include "TX.h"


void*
WF_dispatcher(void* dummy)
{
	printf("WF Dispatcher on\n");
	while(1)
    {
        sleep(1);
    }
}

void
sendMessage(void* msg)
{

}

void*
HW_dispatcher(void*dummy)
{
    printf("HW Dispatcher on\n");
    
    int LengMaster=0;  
    byte *b; 
    
    socket_s* sockfd = newSocket(PORTHW);
    startSocket(sockfd);

    while (1)
    {
        //  Sending========>    ID    sample      // 
       //                       2bytes 2bytes    //  possible changes of these values: timestamp still to be defined // 
      //
     //     Note: add 50ms to each timestamp then send to HW           
        
        if( Self.IsMaster == 1)
        {
            b = (byte*) popFromQueue(&LengMaster,Self.InternalQueue);
            
            if (b == NULL) LengMaster = 0;
            if (LengMaster > 0 )
            {
                sendToSocket(sockfd, b ,sizeof(byte)*4);               
            }
            LengMaster=0; 
        } 
        else 
        {
            printf(" \n I'm a Slave. Nothing to do here.\n");
        }
    }
    free(b);   
    close(sockfd->port); 
}
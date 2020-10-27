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
    
    int tam_master=0;  
    byte *b; 
    
    socket_s* sockfd = newSocket(PORTHW);
    startSocket(sockfd);

    while (1)
    {
        //  Sending========>    ID    sample      //
       //                       2bytes 2bytes    //               
        
        if( Self.IsMaster == 1)
        {
            b = (byte*) popFromQueue(&tam_master,Self.InternalQueue);
            
            if (b == NULL) tam_master = 0;
            if (tam_master > 0 )
            {
                sendToSocket(sockfd, b ,sizeof(byte)*4);               
            }
            tam_master=0; 
        } 
        else 
        {
            printf(" \n I'm a Slave. Nothing to do here.\n");
        }
    }
    free(b);   
    close(sockfd->port); 
}
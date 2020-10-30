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
    byte *ip = (byte*)malloc(sizeof(byte)*TAMIP);
    byte *TimeStamp = (byte*)malloc(sizeof(byte)*TAMTIMESTAMP );
    byte *Sample = (byte*)malloc(sizeof(byte)*TAMSAMPLE);
    byte *TotalSample = (byte*)malloc(sizeof(byte)*TAMTOTALSAMPLE);

    socket_s* sockfd = newSocket(PORTHW);
    startSocket(sockfd);

    while (1)
    {
        //  Sending========>    IP     TimeStamp sample      // 
       //                       2bytes 2bytes    2bytes     //  possible changes of these values // 
      //
     //     Note: add 50ms to each timestamp then send to HW           
        
        if( Self.IsMaster == 1)
        {
            b = (byte*) popFromQueue(&LengMaster,Self.InternalQueue);
            
            if (b == NULL) LengMaster = 0;
            if (LengMaster > 0 )
            {
                // b tem [IP TIMESTAMP  SAMPLE1..SAMPLE2..etc]
                memcpy(ip, b, TAMIP);
                memcpy(TimeStamp, b+TAMIP, TAMTIMESTAMP);

                for(int i=0; i<NUMSAMPLES;i++)
                {
                    memcpy(Sample, b+TAMIP+TAMTIMESTAMP, TAMSAMPLE);
                    //passar agr a info para o outro [IP TIME SAMPLES]
                    memcpy(TotalSample, ip,TAMIP );
                    memcpy(TotalSample+TAMIP,TimeStamp,TAMTIMESTAMP );
                    memcpy(TotalSample+TAMTIMESTAMP+TAMIP, Sample,TAMSAMPLE );
                    
                    sendToSocket(sockfd, TotalSample ,sizeof(byte)*TAMTOTALSAMPLE); 
                }

                // LAST THING TO DO: after sending the first set of N Samples will be incremented 50ms (1/20) to the next N Samples on timestamp //
               //                                                                    and will always repeat this for the next N samples         //

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
=======



void SD_TX(int Sample_Ammount, void* Samples)
{
	return;
}

void PB_TX(byte PBID[2])
{
	return;
}

void PR_TX(byte Originator_IP[2], byte PBID[2], byte SNR)
{
	return;
}

void PC_TX(byte Reached_IP, byte PBID[2], byte SNR)
{
	return;
}

void TA_TX(byte Originator_IP[2], byte PBID[2])
{
	return;
}

void TB_TX(byte PBID[2], timetable* tm, in_message* message)
{
	return;
}

void NE_TX(byte Proxy_IP[2])
{
	return;
}

void NEP_TX(byte Outsiders_IP[2])
{
	return;
}

void NER_TX(byte Outsiders_IP[2])
{
	return;
}

void NEA_TX(byte Outsiders_IP[2], byte PBID[2])
{
	return;
}


>>>>>>> src/TX.c

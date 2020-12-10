#include "queue.h"

void
testQueues()
{
	queue* aq;
	char buff1[24] = "bean good, how bout you";
	char buff2[19] = "hello how you bean";
	char buff3[18] = "very nice, thanks";
	char buff4[14] = "just a packet";
	int dummy;
	char* dummy2;

	printf("Testing queue data structures\n");

	aq	= newQueue();
	addToQueue((void*)buff4, strlen(buff4), aq, -2);
	addToQueue((void*)buff3, strlen(buff3), aq, -1);
	addToQueue((void*)buff1, strlen(buff1), aq, 0);
	addToQueue((void*)buff1, strlen(buff1), aq, 0);
	addToQueue((void*)buff2, strlen(buff2), aq, 10);
	addToQueue((void*)buff4, strlen(buff4), aq, -2);

	printf("Expected output: %s %s %s\n",  buff2, buff2, buff2);
	printf("%s\n%s\n%s\n%s\n%s\n%s\n\n",
					buff2, buff1, buff1,
					buff3, buff4, buff4);

	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);
	
	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);
	
	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);
	
	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);

	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n", dummy2);

	dummy2 = (char*)popFromQueue(&dummy, aq);
	printf("%s\n\n", dummy2);

	delQueue(aq);	
}

#include "WF.h"

void
WF_listener()
{
	char buff[MAXIMUM_PACKET_SIZE*2];		// *2 to prevent overflow on buff+PrevBytes
	int PacketSize;
	int ReadBytes = 0;
	int PrevBytes = 0;
	unsigned int received_messages = 0;
	in_message message;
	timespec res;

	printf("WF Listener on port %u\n", Meta.WF_RX->port);
	startSocket_ws(Meta.WF_RX);

	struct sockaddr_in addr;
	inet_aton("127.0.0.1", &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Meta.WF_RX->port);
	bind(Meta.WF_RX->s,(struct sockaddr*)&addr, sizeof(addr));
	int aux = 1;

	if (setsockopt(Meta.WF_RX->s,SOL_SOCKET,SO_REUSEADDR,&aux,sizeof(int)) == -1)
	{
		perror("Setsockopt");
		exit(1);
	}
	PrevBytes = 0;
	while(1)
	{
		while((ReadBytes = getFromSocket(Meta.WF_RX, buff)) == -1)
		{
			continue;
		}

		clock_gettime(CLOCK_REALTIME, &res);
		printf("\t\t-------Node got message (%d/%d bytes) total of %d!!-------\n", ReadBytes, ++received_messages);

		newInMessage(&message, ReadBytes, buff, res);

		while((ReadBytes = getFromSocket(Meta.WF_RX, &(message.PBE))) == -1)
		{
			continue;
		}

		assert(ReadBytes == sizeof(message.PBE));

		printf("Received full correct message! Received SNR: %u\n", message.PBE);
		printMessage(buff, ReadBytes);
		// Directly handle message
		handler(&message);
		//addToQueue(newInMessage(PacketSize, buff, res), 8, Self.InboundQueue, 1);
	}
}



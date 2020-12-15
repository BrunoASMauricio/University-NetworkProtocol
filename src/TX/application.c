#include "application.h"

void SD_TX(void* buff, int size)
{
	out_message* out_sd = buildSDMessage(buff, size, Self.IP);
	if(!out_sd)
	{
		return;
	}
	addToQueue(out_sd, getPacketSize(buff), Self.OutboundQueue, 1);
}


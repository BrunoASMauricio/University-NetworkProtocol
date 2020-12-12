#include "messages.h"

void newInMessage(in_message* msg, int size, void* buffer, timespec res)
{
	msg->size = size;
	msg->buf = (void*)malloc(size);
	memcpy(msg->buf, buffer, size);
	msg->received_time = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;
}


void clearInMessage(in_message* msg)
{
	free(msg->buf);
}


out_message* newOutMessage(int size, void* buffer)
{
	out_message* ret = (out_message*)malloc(sizeof(out_message));

	ret->size = size;
	ret->buf = (void*)malloc(size);
    memcpy(ret->buf, buffer, size);

	return ret;
}

void delOutMessage(out_message* Message)
{
	if(Message == NULL)
	{
		return;
	}
	
	free(Message->buf);
	free(Message);
}


#include "messages.h"

in_message* newInMessage(int size, void* buffer, timespec res)
{
	in_message* ret = (in_message*)malloc(sizeof(in_message));

	ret->size = size;
	ret->buf = (void*)malloc(size);
	memcpy(ret->buf, buffer, size);
	ret->received_time = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;

	return ret;
}

void delInMessage(in_message* Message)
{
	if(Message == NULL)
	{
		return;
	}

	free(Message->buf);
	free(Message);
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


#ifndef DATA_STRUCTURES_MESSAGE_BUILD
#define DATA_STRUCTURES_MESSAGE_BUILD

#include "../data_structures/node.h"
#include "../utils/messages.c"


void*
/*
* Generates and returns a TB Message
*/
generateTB();

out_message*
buildNEPMessage(byte* SenderIP, byte* OutsiderIP);

out_message*
buildTAMessage(byte* Originator_IP, byte * PBID);

void* buildPRMessage(byte Originator_IP[2], byte PBID[2], float PBE);

out_message*
buildSDMessage(void* buff, int size, byte* IP);

#endif

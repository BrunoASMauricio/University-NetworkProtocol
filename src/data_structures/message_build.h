#ifndef DATA_STRUCTURES_MESSAGE_BUILD
#define DATA_STRUCTURES_MESSAGE_BUILD

#include "../data_structures/node.h"
#include "../utils/messages.c"

// Packet sizes in bytes
// (first -1 is to align the packets type with array index)
const int Packet_Sizes[11] = {-1, 56/8, 56/8, 104/8, 88/8, 144/8, 56/8, 40/8, 40/8, 40/8, 40/8};

enum packet_type{
	SD = 1,
	PB,
	PR,
	PC,
	TB,
	TA,
	NE,
	NEP,
	NER,
	NEA
};

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

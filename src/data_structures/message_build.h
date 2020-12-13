#ifndef DATA_STRUCTURES_MESSAGE_BUILD
#define DATA_STRUCTURES_MESSAGE_BUILD

#include "../data_structures/node.h"
#include "../utils/messages.c"

// Packet sizes in bytes
const int Packet_Sizes[11] = {-1, 56/8, 56/8, 80/8, 64/8, 144/8, 56/8, 40/8, 40/8, 40/8, 40/8};

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

#endif

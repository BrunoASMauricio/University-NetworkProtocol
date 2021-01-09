#include "network.h"

void NE_RX(in_message* msg)
{
	void* message;
    
    byte* Packet = (byte*)msg->buf;
    
    if(Self.IP[0] == Packet[3] && Self.IP[1] == Packet[4])
    {
        //Add the Outsider IP to the Outside-Slaves, updating LastHeard
        byte SenderIP[2];
        SenderIP[0] = Packet[1];
        SenderIP[1] = Packet[2];
        insertSubSlave(SenderIP);
        //insertOutsideSlave(SenderIP);

        unsigned long int Act;
        timespec Res;
        clock_gettime(CLOCK_REALTIME, &Res);
        Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
        
        // se é o master que recebe NE, gera TimeBroadcast e gera NEP 
        // (NEP é sempre resposta de NE)
        if(Self.IsMaster)
        {
			beginTBTransmission();
            NEP_TX(SenderIP);
        }
        else
        {
            insertOutsideSlave(SenderIP);
            NEP_TX(SenderIP);

            // se é um node que não o master, transmite NER
			message = buildNERMessage(Self.Table->begin->Neigh_IP, SenderIP);
			NER_TX(message);
            startRetransmission(rNER, message);
        }
    }
	clearInMessage(msg);
}
void NEP_RX(in_message* msg)
{
    
    byte* Packet = (byte*)msg->buf;
    if(Packet[3] == Self.IP[0] && Packet[4] == Self.IP[1])
    {
        // Cancel retransmission of NE 
        stopRetransmission(rNE);
        // Communication was established with possible proxy
        Self.Status = Waiting;
    }
	clearInMessage(msg);
}

void NER_RX(in_message* msg)
{
	void* message;
	byte* Packet = (byte*)msg->buf;
    
    if(Self.IP[0] == Packet[1] && Self.IP[1] == Packet[2])
    {
        //Add the Outsider IP to the Sub-Slaves, updating LastHeard
        //Add Outsider IP to Pending list
        //Since we delete msg at the end of this
        //function, this: insertSubSlave(&Packet[3]);
        //doesn't work, BIG SAD :( -> memcpy or manual assigned instead
        
        byte SubSlaveIP[2];
        SubSlaveIP[0]= Packet[3];
        SubSlaveIP[1]= Packet[4];
        insertSubSlave(SubSlaveIP);
        insertIPList(Self.OutsidePending, SubSlaveIP);

        unsigned long int Act;
        timespec Res;
        clock_gettime(CLOCK_REALTIME, &Res);
        Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

        table_entry* Outsider = routSearchByIp(Self.Table, SubSlaveIP);
        if(Outsider == NULL)
        {
            printf("IP received in NER not present in rouTable!\n");
            dumpBin((char*)(SubSlaveIP), 2, "Adding new entry with IP:");
            //TODO: Update Last Heard while adding entry
            //routInsertOrUpdateEntry(Self.Table, SubSlaveIP,UNREACHABLE, msg->PBE, 1, msg->received_time);
        }

        if(Self.IsMaster)
        {
            //This assumes generateTB() generates deadline
            //TODO: Check if it does...
			beginTBTransmission();
            
            //Sends NEA Message back
            //Send Outsiders IP and PBID to NEA
            pbid PBID = getNewPBID();
            NEA_TX(&Packet[3], PBID);
        }
        else
        {
            //Transmit the packet up the network
            //Send Outsiders' IP NER_TX
            message = buildNERMessage(Self.Table->begin->Neigh_IP, &Packet[3]);
			NER_TX(message);
            //startRetransmission(rNER, message);
        }
    }
	clearInMessage(msg);
}

void NEA_RX(in_message* msg)
{
    byte* Packet = (byte*)msg->buf;
    byte OutsiderIP[2];

    OutsiderIP[0] = Packet[1];
    OutsiderIP[1] = Packet[2];
    pbid PBID;
    PBID = (0<<4) + Packet[3];
    PBID = Packet[4];

    // se existir outside slave e for o outsider IP, quer dizer que chegamos ao proxy
    if(getOutsideSlave(OutsiderIP))
    {
        stopRetransmission(rNER);
    }
	// se existir na lista de IPs, retiramos para cada node o outsider IP e retransmitimos até chegar ao Proxy
    else if(getIPFromList(Self.OutsidePending, OutsiderIP))
    {
        removeIPList(Self.OutsidePending, OutsiderIP);
        NEA_TX(OutsiderIP, PBID);
    }
	clearInMessage(msg);
}


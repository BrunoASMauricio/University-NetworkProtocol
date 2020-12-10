#include "network.h"

void NE_RX(in_message* msg)
{
	void* message;
    if(msg->buf == NULL)
    {
        printf("msg passed to NE_RX does not have NE packet format!\n");
        return;
    }
    
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
        
        //NOTE(GoncaloX): This is the 1st contact of an new node, as such
        //it shoul be added to the routTable.
        table_entry* Outsider = routInsertOrUpdateEntry(Self.Table, SenderIP, UNREACHABLE, msg->SNR, 1, msg->received_time);
        Outsider->LastHeard = Act;

        // se é o master que recebe NE, gera TimeBroadcast e gera NEP 
        // (NEP é sempre resposta de NE)
        if(Self.IsMaster)
        {
			beginTBTransmission();
            NEP_TX(SenderIP);
        }
        else
        {
            //NOTE(GoncaloX): Maybe this should also happen if node is Master?
            insertOutsideSlave(SenderIP);
            NEP_TX(SenderIP);

            // se é um node que não o master, transmite NER
			message = buildNERMessage(Self.Table->begin->Neigh_IP, SenderIP);
			NER_TX(message);
            startRetransmission(rNER, message);
        }
    }
}
void NEP_RX(in_message* msg)
{
    if(msg->buf == NULL)
    {
        printf("msg passed to NEP_RX does not have NEP packet format!\n");
        return;
    }
    
    byte* Packet = (byte*)msg->buf;
    if(Packet[3] == Self.IP[0] && Packet[4] == Self.IP[1])
    {
        // Cancel retransmission of NE 
        stopRetransmission(rNE);
        // Communication was established with possible proxy
        Self.Status = Waiting;
    }
    delInMessage(msg);
	return;
}

void NER_RX(in_message* msg)
{
	void* message;
	byte* Packet = (byte*)msg->buf;
    if(msg->buf == NULL)
    {
        printf("msg passed to NER_RX does not have NER packet format!\n");
        return;
    }
    
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
            routInsertOrUpdateEntry(Self.Table, SubSlaveIP,UNREACHABLE, msg->SNR, 1, msg->received_time);
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
    
    //Discard packet
    delInMessage(msg);
	return;

}

void NEA_RX(in_message* msg)
{
    if(msg->buf == NULL)
    {
        printf("msg passed to NEA_RX does not have NEA packet format!\n");
        return;
    }
    
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
    else if(getIPFromList(Self.OutsidePending, OutsiderIP) == 0) 
    {
        return;
    }
    else    // se existir na lista de IPs, retiramos para cada node o outsider IP e retransmitimos até chegar ao Proxy
    {
        removeIPList(Self.OutsidePending, OutsiderIP);
        NEA_TX(OutsiderIP, PBID);
    }

	delInMessage(msg);
	return;
}


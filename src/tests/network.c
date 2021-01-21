#include "network.h"

void
testNEP_RX()
{
    printf("\nTesting NEP_RX;\n");
    // Setting some dummy Self values to test 
    Self.IP[0] = 0x03;
    Self.IP[1] = 0x04;

    Self.Status = Outside;
	SETBIT(rNE,Self.Rt.Retransmitables);
    byte dummyPacket[5] = {(PROTOCOL_VERSION<<4) + NEP, 0x01, 0x02, 0x03, 0x04};
    
    timespec Res;
    clock_gettime(CLOCK_REALTIME, &Res);
    
    in_message NEPreceived;
   	newInMessage(&NEPreceived, 5, dummyPacket, Res);
    dumpBin((char*)(NEPreceived.buf), NEPreceived.size, ">>Sent this to NEP_RX: %X");
    NEP_RX(&NEPreceived);
    
    printf("Expected to have STOPPED Retransmission on NE\n");
    printf("Checking if Self.Rt.Retransmitables is set on NE:\n");
    printf("Self.Rt.Retransmitables on NE:%s\n", 
            CHECKBIT(rNE, Self.Rt.Retransmitables) ? "YES\n": "NO\n");
    
    printf("Self.Status after NEP_RX\n"
            "Expected Waiting -> %d\n", Waiting);
    printf("Self.Status = %d\n", Self.Status);
    
    printf("\nFinished NEP_RX;\n");
}

void
testBuildNE()
{
    byte SourceIP[2] = {0x01, 0x02};
    byte DestIP[2] = {0x03, 0x04};
    void* NEpacket = buildNEMessage(SourceIP ,DestIP);
    printf("\nTesting buildNEMessage;\n Expected output:\n");
    printf(">>NEpacket: 5 0x%X 0x01 0x02 0x03 0x04\n", 
                                (PROTOCOL_VERSION<<4)+NE);
    printf("Actual output:\n");
    dumpBin((char*)NEpacket, getPacketSize(NEpacket), ">>NEpacket: %X ");
    printf("\nFinished testing BuildNE;\n");
}

void
testBuildNEP()
{
    byte SourceIP[2] = {0x01, 0x02};
    byte DestIP[2] = {0x03, 0x04};
    out_message* NEPpacket = buildNEPMessage(SourceIP ,DestIP);
    printf("\nTesting buildNEPMessage;\n Expected output:\n");
    printf(">>NEPpacket: 5 0x%X 0x01 0x02 0x03 0x04\n", 
                                (PROTOCOL_VERSION<<4)+NEP);
    printf("Actual output:\n");
    dumpBin((char*)(NEPpacket->buf), NEPpacket->size, ">>NEPpacket: %X ");
    printf("\nFinished testing BuildNEP;\n");
}
    

void
testNER_RX()
{
    printf("\nTesting NER_RX;\n");
    // Setting some dummy Self values to test 
    Self.SubSlaves = newIPList();
    Self.OutsideSlaves = newIPList();
    Self.OutsidePending = newIPList();
    Self.Table = routNewTable();
    Self.InboundQueue = newQueue();
    Self.OutboundQueue = newQueue();
    
    Self.IP[0] = 0x03;
    Self.IP[1] = 0x04;
    byte dummyPacket[5] = {(PROTOCOL_VERSION<<4) + NER, 
                           0x03, 0x04, 0x01, 0x02};
    routInsertOrUpdateEntry(Self.Table, &dummyPacket[3], UNREACHABLE, 1, 1, 1, 1);
    timespec Res;
    clock_gettime(CLOCK_REALTIME, &Res);
    
    in_message NERreceived;
   	newInMessage(&NERreceived, 5, dummyPacket, Res);
    dumpBin((char*)(NERreceived.buf), NERreceived.size, ">>Sent this to NEP_RX: %X");
    NER_RX(&NERreceived);

    printf("\nExpected to have added IP 0x01 0x02 as SubSlave:\n");
    byte SubSlaveIP[2] = {0x01, 0x02};
    printf("Is SubSlave present? %s",
            getIPFromList(Self.SubSlaves, SubSlaveIP) ? "YES\n": "NO\n");
    
    printf("Expected to have updated LastHeard on routTable\n");
    table_entry* Outsider = routSearchByIp(Self.Table, &dummyPacket[3]);
    printf("Current LastHeard: %lu\n", Outsider->LastHeard);
    
    if(Self.IsMaster)
    {
        printf("MASTER CASE:\n");
        printf("Expected to have generated TB\n");
        printf("Expected to have sent NEA\n");
    }
    else
    {
        printf("SLAVE CASE:\n");
        printf("Expected to have started startRetransmission on NER\n");
        printf("Checking if Self.Rt.Retransmitables is set on NER:\n");
        printf("Self.Rt.Retransmitables on NER:%s\n", 
                CHECKBIT(rNER, Self.Rt.Retransmitables) ? "YES\n": "NO\n");
    }
    
    routRemoveEntry(Self.Table, SubSlaveIP);
    Self.Table = NULL;
    delQueue(Self.InboundQueue);
    delQueue(Self.OutboundQueue);
    
    printf("\nFinished NER_RX;\n");
}

void
testNEA_RX()
{
	printf("\nTesting NEA_RX;\n");
    // Setting some dummy Self values to test 
    Self.OutsideSlaves = newIPList();
    Self.OutsidePending = newIPList();
    Self.OutboundQueue = newQueue();
    SETBIT(rNER, Self.Rt.Retransmitables);
    
    byte OutsiderIP[2];
    OutsiderIP[0] = 0x03;
    OutsiderIP[1] = 0x04;
    byte dummyPacket[5] = {(PROTOCOL_VERSION<<4) + NEA, 
                           0x03, 0x04, 0x01, 0x02};

    timespec Res;
    clock_gettime(CLOCK_REALTIME, &Res);
    
    printf("\nTesting direct connection to OutsideSlave 1st\n");
    insertOutsideSlave(OutsiderIP);
    in_message NEAreceived;
   	newInMessage(&NEAreceived, 5, dummyPacket, Res);
    NEA_RX(&NEAreceived);
    
    printf("Expected to have stoped startRetransmission on NER\n");
    printf("Self.Rt.Retransmitables on NER:%s\n", 
            CHECKBIT(rNER, Self.Rt.Retransmitables) ? "YES\n": "NO\n");
    
    removeOutsideSlave(OutsiderIP);
    printf("\nNow testing indirect connection to OutsideSlave\n");
    
    insertIPList(Self.OutsidePending, OutsiderIP);
    printf("getIPFromList got: %d\n", getIPFromList(Self.OutsidePending, OutsiderIP));
   	newInMessage(&NEAreceived, 5, dummyPacket, Res);
    NEA_RX(&NEAreceived);
    printf("Expected to have removed OutsiderIP from OutsiderPending\n");
    printf("getIPFromList got: %d\n", getIPFromList(Self.OutsidePending, OutsiderIP));
    
    delIPList(Self.SubSlaves);
    Self.SubSlaves = NULL;
    delIPList(Self.OutsideSlaves);
    Self.OutsideSlaves = NULL;

    delQueue(Self.OutboundQueue);
    return;
}


void
testBuildNER()
{
    byte NextHopIP[2] = {0x01, 0x02};
    byte OutsiderIP[2] = {0x03, 0x04};
    void* NERpacket = buildNERMessage(NextHopIP ,OutsiderIP);
    printf("\nTesting buildNERMessage.\n Expected output:\n");
    printf(">>NERpacket: 5 0x%X 0x01 0x02 0x03 0x04\n", 
                                (PROTOCOL_VERSION<<4)+NER, NextHopIP, OutsiderIP );
    printf("Actual output:\n");
    dumpBin((char*)NERpacket, getPacketSize(NERpacket), ">>NERpacket: %X ");
    printf("\nFinished testing BuildNER;\n");
}

void
testBuildNEA()
{
    byte OutsiderIP[2] = {0x01, 0x02};
    pbid PBID = 0;
    out_message* NEApacket = buildNEAMessage(OutsiderIP, PBID);
    printf("\nTesting buildNEAMessage.\n Expected output:\n");
    printf(">>NEApacket: 5 0x%X 0x01 0x02 %X %X\n", 
                                (PROTOCOL_VERSION<<4)+NEA, 
                                (PBID >> 8) & 0xff),
                                (PBID & 0xff);
    printf("Actual output:\n");
    dumpBin((char*)(NEApacket->buf), NEApacket->size, ">>NEApacket: %X ");
    printf("\nFinished testing BuildNEA;\n");
}


void
testNE_RX()
{
    printf("\nTesting NE_RX;\n");
    // Setting some dummy Self values to test 
    Self.SubSlaves = newIPList();
    Self.OutsideSlaves = newIPList();
    Self.Table = routNewTable();
    Self.InboundQueue = newQueue();
    Self.OutboundQueue = newQueue();
    
    Self.IP[0] = 0x03;
    Self.IP[1] = 0x04;
    Self.Status = Inside;
    byte dummyPacket[5] = {(PROTOCOL_VERSION<<4) + NE, 0x01, 0x02, 0x03, 0x04};
    
    timespec Res;
    clock_gettime(CLOCK_REALTIME, &Res);
    
    in_message NEreceived;
   	newInMessage(&NEreceived, 5, dummyPacket, Res);
    dumpBin((char*)(NEreceived.buf), NEreceived.size, ">>Sent this to NE_RX: %X");
    NE_RX(&NEreceived);

    printf("\nExpected to have added IP 0x01 0x02 as SubSlave:\n");
    byte OutsideSlaveIP[2] = {0x01, 0x02};
    printf("Is SubSlave present? %s",
            getIPFromList(Self.SubSlaves, OutsideSlaveIP) ? "YES\n": "NO\n");
    
    if(Self.IsMaster)
    {
        printf("\nMASTER CASE:\n");
        printf("Expected to have generated TB\n");
        printf("Expected to have sent NEP\n");
    }
    else
    {
        printf("\nSLAVE CASE:\n");
        printf("Expected to have sent NEP and NER\n");
       
        printf("Expected to have started Retransmission of NER\n");
        printf("Checking if Self.Rt.Retransmitables is set on NER:\n");
        printf("Self.Rt.Retransmitables on NER:%s\n", 
                CHECKBIT(rNER, Self.Rt.Retransmitables) ? "YES\n": "NO\n");
    }

    delIPList(Self.SubSlaves);
    Self.SubSlaves = NULL;
    delIPList(Self.OutsideSlaves);
    Self.OutsideSlaves = NULL;

    routRemoveEntry(Self.Table, OutsideSlaveIP);
    Self.Table = NULL;
    delQueue(Self.InboundQueue);
    delQueue(Self.OutboundQueue);
    
    printf("\nFinished NE_RX;\n");
}

#include "test.h"

void
testAll(){
	char a[6];
	a[0] = 0xaf;
	a[1] = 0x99;
	a[2] = 0x82;
	a[3] = 0xff;
	a[4] = 0x00;
	a[5] = 0x11;

	printf("Starting protocol test\n---------\n");

	printf("Testing printfLog. Expected output:\n");
	printf(">>1 teste 0.5\n");
	printf(">>%d teste %0.1f\n", 1, 0.5);

	printf("Testing dumpBin. Expected output:\n");
	printf(">>Hello 4: 0xaf 0x99 0x82 0xff 0x00 0x11\n");
	
	dumpBin(a, sizeof(a), ">>Hello %d: ", 4);

	testQueues();
	
	testRoutingTable();

    testBuildNE();
    testNE_RX();
	
    testBuildNEP();
    testNEP_RX();

    testBuildNER();
    testNER_RX();
    
	testTimeTable();

	testLists();

	testIPLists();

    testBuildNEA();
    testNEA_RX();
    
    testLists();

	testTB();

	testBuildTA();
	testTA_RX();


	printf("Iterative testing of PBID-IP pair table\n");
	test_PBID_IP_table();

	printf("Ending protocol test\n---------\n");
	printf("Starting protocol measurements\n---------\n");
	performMeasurements();
	printf("Ending protocol measurements\n---------\n");
}

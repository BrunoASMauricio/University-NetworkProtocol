#include "list.h"


void
testIPLists()
{
	IPList* IPL = newIPList();
	byte IP0[2] = {1,2};
	byte IP1[2] = {3,4};
	byte IP2[2] = {5,6};

	printf("Testing IP Lists\n");
	printf("Got:\n");
	printf("%d\n", getIPFromList(IPL, IP0));
	insertIPList(IPL, IP0);
	insertIPList(IPL, IP0);
	printf("%d\n", getIPFromList(IPL, IP0));
	insertIPList(IPL, IP1);
	removeIPList(IPL, IP0);
	printf("%d ", IPL->L->Size);
	printf("%d ", getIPFromList(IPL, IP1));
	printf("%d\n", getIPFromList(IPL, IP0));
	printf("Expected:\n0\n1\n1 1 0\n");
	delIPList(IPL);
}
void
testLists()
{
	List* L = newList();
	printf("Testing lists\n");
	char buff0[10] = "EL0";
	char buff1[10] = "EL1";
	char buff2[10] = "EL2";
	insertInList(L, buff0, -1);
	insertInList(L, buff1, 1);
	insertInList(L, buff2, 5);
	removeFromList(L, 1);
	removeFromList(L, 1);
	removeFromList(L, 1); // These two should not work because now there is
	removeFromList(L, 1); // only 1 element (position 0)
	printf("Got:\n");
	printList(L);
	printf("Expected:\nList size: 1\nEL0\n");
	delList(L);
}

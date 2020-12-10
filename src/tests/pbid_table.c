#include "pbid_table.h"

void
test_PBID_IP_table()
{
	byte* etcI = (byte*)malloc(2*sizeof(byte));
	byte* etcP = (byte*)malloc(2*sizeof(byte));

	pbid_ip_table* lmao = pbidInitializeTable();

	int choice;

	while(1)
	{
		printf("\nPress\n0 to insert pair\t1 to search pair\t2 to remove pair\t9 to end test\n");
		scanf("%d", &choice);

		if(choice == 0)
		{
			printf("\ninsert IP to add (2 bytes in hex - something like 19a5)\n");
			scanf("%2hhx%2hhx", &etcI[1], &etcI[0]);
			printf("insert PBID to add (2 bytes in hex)\n");
			scanf("%2hhx%2hhx", &etcP[1], &etcP[0]);


			pbidInsertPair(etcI, etcP, lmao);

			printf("Printing table\n");
			pbidPrintTable(lmao);
		}
		else if(choice == 1)
		{
			printf("\ninsert IP of pair to search for (2 bytes in hex - something like 19a5)\n");
			scanf("%2hhx%2hhx", &etcI[1], &etcI[0]);
			printf("insert PBID of pair to search for (2 bytes in hex)\n");
			scanf("%2hhx%2hhx", &etcP[1], &etcP[0]);

			if(pbidSearchPair(etcI, etcP, lmao))
				printf("FOUND!\n");
			else
				printf("not found :(\n");
		}
		else if(choice == 2)
		{
			printf("\ninsert IP of pair to be removed (2 bytes in hex - something like 19a5)\n");
			scanf("%2hhx%2hhx", &etcI[1], &etcI[0]);
			pbidRemovePair(etcI, lmao);

			printf("Printing updated table\n");
			pbidPrintTable(lmao);
		}
		else if(choice == 9)
			break;
	}
}

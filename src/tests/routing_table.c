#include "routing_table.h"

void 
testRoutingTable()
{
    table *Tbl;
	timespec Res;
	unsigned long int Act;

	
    Tbl=routNewTable();

    printf("Entry with lowest distance must be on the top...\nPlease check if the table size corresponds with the announced one\n");

    printf("Adding a 1st entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    
    
    byte IP[2]={2,3};
    short Qual = 8888;
    short Avg = 222;
    short Eff = 777;

    table_entry *Entry;
    Entry=routInsertOrUpdateEntry(Tbl,IP,Qual,Avg,Eff, Act, 1);
    if(Entry==NULL) printf("Failed to insert entry\n");

    routPrintTableContent(Tbl);
    printf("\n\n");
    
    printf("Adding a 2nd entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    byte IP_[2]={4,4};
    short Qual1 = 88;
    short Avg1 = 222;
    short Eff1 = 777;

     
    table_entry *Entry1;
    Entry1=routInsertOrUpdateEntry(Tbl, IP_, Qual1, Avg1, Eff1, Act, 1);

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 3rd entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    byte IP_0[2]={7,8};
    short Qual2 = 1000;
    short Avg2 = 222;
    short Eff2 = 777;

    table_entry *Entry2;
    Entry2=routInsertOrUpdateEntry(Tbl, IP_0, Qual2, Avg2, Eff2, Act, 1);

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 4th entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    byte IP_1[2]={1,1};
    short Qual3 = 10000;
    short Avg3 = 222;
    short Eff3 = 777;

    table_entry *Entry3;
    Entry3=routInsertOrUpdateEntry(Tbl, IP_1, Qual3, Avg3, Eff3, Act, 1);  

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Adding a 5th entry...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    byte IP_2[2]={2,2};
    short Qual4 = 2000;
    short Avg4 = 222;
    short Eff4 = 777;

    table_entry *Entry4;
    Entry4=routInsertOrUpdateEntry(Tbl, IP_2, Qual4, Avg4, Eff4, Act, 1);  

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Updating entry with IP: 11 and changed last heard...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    

    short QualUpd = 1;
    routInsertOrUpdateEntry(Tbl, IP_1, QualUpd, Avg4, Eff4, Act, 1);  

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Updating entry with IP: 44 ...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    
    short QualUpd_1 = 100;
    routInsertOrUpdateEntry(Tbl, IP_, QualUpd_1, Avg4, Eff4,Act, 1);  

    routPrintTableContent(Tbl);
    printf("\n\n");

    printf("Updating entry with IP: 11 ...\n");
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
    
    short QualUpd_2 = 1500;
    routInsertOrUpdateEntry(Tbl, IP_1, QualUpd_2, Avg4, Eff4, Act, 1);  

    routPrintTableContent(Tbl);
    printf("\n\n");

	printf("Getting first position... \n");
	table_entry* entry5;
	entry5= routGetEntryByPos(Tbl, 1);

	printf("1st entry distance : %hi, 1st entry NextHop_IP: %d%d\n", entry5->Distance, entry5->Neigh_IP[0],entry5->Neigh_IP[1] );

	printf("Getting second position... \n");
	entry5= routGetEntryByPos(Tbl, 2);
	printf("2nd entry distance : %hi, 2nd entry NextHop_IP: %d%d\n", entry5->Distance, entry5->Neigh_IP[0],entry5->Neigh_IP[1] );
}



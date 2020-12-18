#ifndef DATA_STRUCTURES_NODE
#define DATA_STRUCTURES_NODE

#include "../data_structures/routing_table.h"
#include "../data_structures/retransmission.h"

#define UNSET 255

// Unnecessary
#define SAMPLE_SIZE 16
#define MAX_TRANS_SIZE 4496



#define DEFAULT_HW_PORT 901
#define DEFAULT_WS_PORT 23092
#define DEFAULT_WF_TX_PORT 903
#define DEFAULT_WF_RX_PORT 904



typedef struct{
	pthread_t WF_listener_t;
	pthread_t WF_dispatcher_t;
	pthread_t WS_listener_t;
	pthread_t HW_dispatcher_t;
	pthread_t Retransmission_t;
	pthread_t Main_t;

	socket_s* WF_RX;
	socket_s* WF_TX;
	bool Post;
	bool Debug;
	bool Quiet;
	
	int WF_TX_port;
	int WF_RX_port;
	int HW_port;
	int WS_port;
} meta_data;

enum node_status{
	Outside,
	Waiting,
	Inside,
	NA, //to use in the master one
};


/*
 * Internal queue is only handled by the WS and HW interfaces
 */
typedef struct{
	byte IsMaster;
	queue* OutboundQueue;
	queue* InboundQueue;
	queue* InternalQueue;
	byte IP[2];
	table* Table;
	timetable* TimeTable;
	timetable* NewTimeTable;
	pbid_ip_table* PBID_IP_TA;
	bool SyncTimestamp;
	retransmission Rt;
	IPList* SubSlaves;
	IPList* OutsideSlaves;
	IPList* OutsidePending;
	byte TB_PBID[2];
	node_status Status;
	pbid_ip_table* RoutingPBIDTable;
    pbid PBID;
	// ...
} node;


meta_data Meta;
node Self;
byte  *ip;


/*
 * Performs a network test
 * sets Self.isMaster to the best guessed value
 * (has an internet connection)
 * TODO: real check is to check HW connection
 */
void
setMaster();

/*
 * Returns the size of the packet in buf, in bytes
 * Returns -1 on undefine message type/wrong version
 */
int
getPacketSize(void* buf);

/*
 * snippet code provided by anand choubey, consulted on:
 * https://www.codeproject.com/Articles/35103/Convert-MAC-Address-String-into-Bytes
* converts mac address string to bytes (does hex to bytes conversion)
*/
unsigned char* ConverMacAddressStringIntoByte(const char *pszMACAddress, unsigned char* pbyAddress);

byte*
getIP();
	
/*
 * Increments and returns global PBID 
 */
pbid
getNewPBID();

#endif

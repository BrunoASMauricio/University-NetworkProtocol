#include "main.h"
#include "data.h"
#include "routing_table.h"

/*
 * Main function
 * Parses the command line arguments (sets verbosity and debugging)
 * Starts the main protocol loop
 */
int
main(int argc, char **argv)
{
	int c;

	Meta.Debug = false;
	Meta.Quiet = false;
	Meta.Post= false;
	Meta.Log = NULL;
	Self.IsMaster = UNSET;
	Self.IP[0] = 0xff;
	Self.SyncTimestamp = true;
	Self.Rt.Retransmitables = 0;

	Meta.HW_port = DEFAULT_HW_PORT;
	Meta.WS_port = DEFAULT_WS_PORT;
	Meta.WF_TX_port = DEFAULT_WF_TX_PORT;
	Meta.WF_RX_port = DEFAULT_WF_RX_PORT;

	while (1)
    {
		int option_index = 0;
		static struct option long_options[] = {
			{"log",		no_argument,		0, 'l'},
			{"post",	no_argument,		0, 'p'},
			{"debug",	no_argument,		0, 'd'},
			{"quiet",	no_argument,		0, 'q'},
			{"HW",		required_argument,		0, 'H'},
			{"WS",		required_argument,		0, 'W'},
			{"WF_TX",	required_argument,		0, 'T'},
			{"WF_RX",	required_argument,		0, 'R'},
			{"IP",		required_argument,			0, 'I'},
			{0,			0,					0,  0 }
		};

		c = getopt_long(argc, argv, "qlpdr:H:W:T:R:I:s", long_options, &option_index);

		if (c == -1)	break;

		switch(c) {
			case 'r':
				if (optarg[0] == 'M')
                {
					Self.IsMaster = true;
					printf("Forcing node to master\n");
				}
                else if (optarg[0] == 'S')
                {
					Self.IsMaster = false;
					printf("Forcing node to slave\n");
				}
                else
                {
					fatalErr("Node can only be master (M) or slave (S)\n");
				}
				break;

			case 'l':
				if((Meta.Log = fopen("./log", "a")) == NULL)
                {
					fatalErr("Could not open log. Errno set to: %d\n", errno);
				}
				break;

			case 'd':
				Meta.Debug = true;
				break;

			case 'q':
				Meta.Quiet = true;
				break;

			case 'p':
				Meta.Post= true;
				break;

			case 'H':
				Meta.HW_port = atoi(optarg);
				break;
			case 'W':
				Meta.WS_port = atoi(optarg);
				break;
			case 'T':
				Meta.WF_TX_port = atoi(optarg);
				break;
			case 'R':
				Meta.WF_RX_port = atoi(optarg);
				break;
			case 'I':
				Self.IP[0] = 0x82;
				Self.IP[1] = atoi(optarg);
				break;
			case 's':
				Self.SyncTimestamp = true;
				break;
			case '?':
				fatalErr("Undefined argument\n");
				break;
			default:
				printf("?? getopt returned character code 0%o ??\n", c);
		}
	}

	printf("Starting protocol\n");	
	printf("Quiet: %d\n", Meta.Quiet);
	printf("Configured ports:\n");
	printf("HW: %d\n", Meta.HW_port);
	printf("WS: %d\n", Meta.WS_port);
	printf("WF_TX: %d\n", Meta.WF_TX_port);
	printf("WF_RX: %d\n", Meta.WF_RX_port);
	if(Self.IP[0] != 0xff)
	{
		printf("Configured with IP %d %d\n", Self.IP[0], Self.IP[1]);
	}
	if(Self.SyncTimestamp)
	{
		printf("We are in the matrix\n");
	}

	
	if (Meta.Post)
    {
        testAll();
    }
	
	setup();

	handler();

	clean();

	exit(EXIT_SUCCESS);
}

void
setup()
{
	int rc;
	//memcpy(Self.IP,getIP(),sizeof(Self.IP)); //setting IP
	setMaster();

	Self.OutboundQueue = newQueue();
	Self.InboundQueue = newQueue();
	Self.InternalQueue = newQueue();
	Self.Table = routNewTable();
	Self.SubSlaves = newIPList();
	Self.OutsideSlaves= newIPList();
	Self.TimeTable = NULL;

	if (pthread_mutex_init(&(Self.Rt.Lock), NULL) != 0)
    {
        fatalErr("mutex init failed for outbound lock\n");
    }


	Meta.WF_RX = newSocket(Meta.WF_RX_port);
	startSocket(Meta.WF_RX);

	Meta.WF_TX = newSocket(Meta.WF_TX_port);
	startSocket(Meta.WF_TX);

	if (rc = pthread_create(&(Meta.WF_listener_t), NULL, WF_listener, NULL))
    {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}

	if (rc = pthread_create(&(Meta.WF_dispatcher_t), NULL, WF_dispatcher, NULL))
    {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}
	
	if (rc = pthread_create(&(Meta.WS_listener_t), NULL, WS_listener, NULL))
    {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}

	if (rc = pthread_create(&(Meta.HW_dispatcher_t), NULL, HW_dispatcher, NULL))
    {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}

	if (rc = pthread_create(&(Meta.Retransmission_t), NULL, retransmit, NULL))
	{
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}
}


void testSim()
{
	int size = sizeof(byte)*7;
	byte* PBPacket = (byte*)malloc(size);
    short MasterDistance = 403;

    if(PBPacket == NULL)
	{
        fatalErr("Couldn't assign memory to PB Packet \n");
    }

    PBPacket[0]=(PROTOCOL_VERSION<<4)+2;
    PBPacket[1]=Self.IP[0];
    PBPacket[2]=Self.IP[1];
    PBPacket[3]= (238>> 8) &0xff;
    PBPacket[4]= 238 &0xff ;

	PBPacket[5]= (MasterDistance >> 8) &0xff;
	PBPacket[6]= MasterDistance &0xff;


	out_message* message = newOutMessage(size, PBPacket);
	//dumpBin((char*)(PBPacket), size, "Sent PB message (size %d) to port %u: ", sendToSocket(Meta.WF_TX, PBPacket, size), Meta.WF_TX->port);
    addToQueue(message, sizeof(message), Self.OutboundQueue, 1);

    free(PBPacket); //COULD SOMEONE CHECK IF THIS FREE MAKES SENSE?

	return;
}
void
handler()
{
	in_message* Message;

	while (1)
    {
		testSim();
		usleep(300000); // Ever 300ms (0.3s)
		//usleep(100000);
		continue;
		Message = getMessage();
		if(Message == NULL) continue;
		switch (((byte*)Message)[0])
        {
			case SD:
				SD_RX(Message);
				break;
			case PB:
				PB_RX(Message);
				break;
			case PR:
				PR_RX(Message);
				break;
			case PC:
				PC_RX(Message);
				break;
			case TA:
				TA_RX(Message);
				break;
			case TB:
				TB_RX(Message);
				break;
			case NE:
				NE_RX(Message);
				break;
			case NEP:
				NEP_RX(Message);
				break;
			case NER:
				NER_RX(Message);
				break;
			case NEA:
				NEA_RX(Message);
				break;
			default:
				printf("Unrecognized Message type %d\n", ((byte*)Message)[0]);
		}
		free(Message);
	}
}

void
clean()
{
	if(Meta.WF_TX->s != -1)
	{
		close(Meta.WF_TX->s);
	}
	if(Meta.WF_RX->s != -1)
	{
		close(Meta.WF_RX->s);
	}
	if(Meta.Log)
    {
        fclose(Meta.Log);
    }
}


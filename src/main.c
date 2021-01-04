#include "main.h"

/*
 * Main function
 * Parses the command line arguments (sets verbosity and debugging)
 * Starts the main protocol loop
 */
int
main(int argc, char **argv)
{
	int c;
	char buf[30];

	Meta.Debug = false;
	Meta.Quiet = false;
	Meta.Post= false;
	Self.IsMaster = UNSET;
	Self.IP[0] = 0xff;
	Self.SyncTimestamp = false;
	Self.Rt.Retransmitables = 0;

	Meta.HW_port = DEFAULT_HW_PORT;
	Meta.WS_port = DEFAULT_WS_PORT;
	Meta.WF_TX_port = DEFAULT_WF_TX_PORT;
	Meta.WF_RX_port = DEFAULT_WF_RX_PORT;

	signal(SIGINT, clean);

	// Setting IP
	memcpy(Self.IP, getIP(), sizeof(Self.IP));

	while (1)
    {
		int option_index = 0;
		static struct option long_options[] = {
			{"post",	no_argument,		0, 'p'},
			{"debug",	no_argument,		0, 'd'},
			{"quiet",	no_argument,		0, 'q'},
			{"Sim",		no_argument,		0, 's'},
			{"HW",		required_argument,		0, 'H'},
			{"WS",		required_argument,		0, 'W'},
			{"WF_TX",	required_argument,		0, 'T'},
			{"WF_RX",	required_argument,		0, 'R'},
			{"IP",		required_argument,			0, 'I'},
			{0,			0,					0,  0 }
		};

		c = getopt_long(argc, argv, "qpdr:H:W:T:R:I:s", long_options, &option_index);

		if (c == -1)	break;

		switch(c) {
			case 'r':
				if (optarg[0] == 'M')
                {
					Self.IsMaster = true;
				}
                else if (optarg[0] == 'S')
                {
					Self.IsMaster = false;
				}
                else
                {
					fatalErr("Node can only be master (M) or slave (S)\n");
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
				fatalErr("?? getopt returned character code 0%o ??\n", c);
		}
	}
	
	// Identifies the main thread
	// This needs to be here, so that testing output is correctly identified
	Meta.Main_t = pthread_self();
	fprintf(stdout, "%u\n", (Self.IP[0]<<8) | Self.IP[1]);
	fprintf(stdout, "%d\n", getpid());
	fflush(stdout);

	printf("Starting protocol\n");	
	printf("Quiet: %d\n", Meta.Quiet);
	printf("Configured ports:\n");
	printf("HW: %d\n", Meta.HW_port);
	printf("WS: %d\n", Meta.WS_port);
	printf("WF_TX: %d\n", Meta.WF_TX_port);
	printf("WF_RX: %d\n", Meta.WF_RX_port);

	// Print IP if it was hardcoded
	if(Self.IP[0] != 0xff)
	{
		printf("Configured with IP 0x%02hhx 0x%02hhx\n", Self.IP[0], Self.IP[1]);
	}

	// Are we in the WF simulation? (sending timestamps)
	if(Self.SyncTimestamp)
	{
		printf("We are in the matrix\n");
	}

	// Power On Self Test?
    if (Meta.Post)
    {
        testAll();
    }
	
	// Setup data structures and threads
	setup();

	// Master immediately starts transmitting PBs
	if(Self.IsMaster)
	{
		startRetransmission(rPB, createPB());
	}

	// Main thread runs the WF listener directly
	WF_listener();
}

void
setup()
{
	int rc;

	setMaster();

	if(Self.IsMaster)
	{
		// Setting node as the Master
		Self.Status = NA;
		// TB starts at 1 in the Master, so Slaves are "behind" it
		((short*)Self.TB_PBID)[0] = 1;
		printf("I am the Master!\n");
	}
	else
	{
		// Setting node as outside node
		Self.Status = Outside;
		((short*)Self.TB_PBID)[0] = 0;
		printf("I am a Slave!\n");
	}
	
    Self.PBID = 0;
	Self.OutboundQueue = newQueue();
	Self.InboundQueue = newQueue();
	Self.InternalQueue = newQueue();
	Self.Table = routNewTable();
	Self.PBID_IP_TA = pbidInitializeTable();
	Self.SubSlaves = newIPList();
	Self.OutsideSlaves= newIPList();
	Self.RoutingPBIDTable = pbidInitializeTable();
	Self.OutsidePending= newIPList();
	Self.TimeTable = newTimeTable();
	Self.NewTimeTable = newTimeTable();

	if (pthread_mutex_init(&(Self.Rt.Lock), NULL) != 0)
    {
        fatalErr("mutex init failed for outbound lock\n");
    }

	if (pthread_mutex_init(&(Self.OutputLock), NULL) != 0)
    {
        fatalErr("mutex init failed for outbound lock\n");
    }


	Meta.WF_RX = newSocket(Meta.WF_RX_port);
	startSocket(Meta.WF_RX);

	Meta.WF_TX = newSocket(Meta.WF_TX_port);
	startSocket(Meta.WF_TX);

	// WF message sender
	if (rc = pthread_create(&(Meta.WF_dispatcher_t), NULL, WF_dispatcher, NULL))
    {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}
	
	// WS data payload receiver
	if (rc = pthread_create(&(Meta.WS_listener_t), NULL, WS_listener, NULL))
    {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}

	// HW data payload sender
	if (rc = pthread_create(&(Meta.HW_dispatcher_t), NULL, HW_dispatcher, NULL))
    {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}

	// Retransmissions responsible thread
	if (rc = pthread_create(&(Meta.Retransmission_t), NULL, retransmit, NULL))
	{
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}
}


void
handler(in_message* _Message)
{
	in_message* Message = (in_message*)_Message;
	switch (((byte*)(Message->buf))[0] & 0x0f)
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
			printf("Unrecognized Message type %d\n", ((byte*)Message->buf)[0] & 0x0f);
	}
}

void
clean(int signo)
{
	clean();
	exit(0);
}

void
clean()
{
	printf("\nShutting down node\n");
	fflush(stdout);
	fflush(stderr);
	if(Meta.WF_TX->s != -1)
	{
		close(Meta.WF_TX->s);
	}
	if(Meta.WF_RX->s != -1)
	{
		close(Meta.WF_RX->s);
	}
}


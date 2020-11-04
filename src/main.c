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
	Meta.Post= false;
	Meta.Log = NULL;
	Self.IsMaster = UNSET;

	while (1) 
    {
		int option_index = 0;
		static struct option long_options[] = {
			{"log",		no_argument,		0, 'l'},
			{"post",	no_argument,		0, 'p'},
			{"debug",	no_argument,		0, 'd'},
			{0,			0,					0,  0 }
		};

		c = getopt_long(argc, argv, "lpdr:", long_options, &option_index);

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

			case 'p':
				Meta.Post= true;
				break;

			case '?':
				fatalErr("Undefined argument\n");
				break;

			default:
				printf("?? getopt returned character code 0%o ??\n", c);
		}
	}

	printf("Starting protocol\n");	
	
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
	Self.RoutingPBID=1;

	if(Self.IsMaster==false) //setting node as an outside one
	{
		Self.Status=Outside;
	}
	else
	{
		Self.Status=NA;
	}
	

	Self.OutboundQueue = newQueue();
	Self.InboundQueue = newQueue();
	Self.InternalQueue = newQueue();
	Self.Table= routNewTable();

	Meta.Input_socket = newSocket(INBOUND_PORT);
	startSocket(Meta.Input_socket);

	Meta.Output_socket = newSocket(OUTBOUND_PORT);
	startSocket(Meta.Output_socket);

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
}

void
handler()
{
	in_message* Message;

	while (1)
    {
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
	if(Meta.Input_socket->s != -1)
	{
		close(Meta.Input_socket->s);
	}
	if(Meta.Output_socket->s != -1)
	{
		close(Meta.Input_socket->s);
	}
	if(Meta.Log)
    {
        fclose(Meta.Log);
    }
}


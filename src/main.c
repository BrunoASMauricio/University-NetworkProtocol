#include "main.h"

int
/*
 * Main function
 * Parses the command line arguments (sets verbosity and debugging)
 * Sets up the correct protocol version
 * Starts the main protocol loop
 */
main(int argc, char **argv)
{
	int c;

	//meta.version = 1;
	meta.debug = false;
	meta.post= false;
	meta.log = NULL;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			//{"version",	required_argument,	0, 'v'},
			{"log",		no_argument,		0, 'l'},
			{"post",	no_argument,		0, 'p'},
			{"debug",	no_argument,		0, 'd'},
			{0,			0,					0,  0 }
		};
		// v:
		c = getopt_long(argc, argv, "lpd", long_options, &option_index);

		if (c == -1)	break;

		switch (c) {
			/*
			case 'v':
				if (strcmp(long_options[option_index].name, "version") == 0) {
					meta.version = atoi(optarg);
				}
				break;
			*/

			case 'l':
				if ((meta.log = fopen("./log", "a")) == NULL) {
					printfErr("Could not open log. Errno set to: %d\n", errno);
					exit(-1);
				}
				break;

			case 'd':
				meta.debug = true;
				break;

			case 'p':
				meta.post= true;
				break;

			case '?':
				break;

			default:
				printf("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (optind < argc) {
		while(optind < argc){
			printfErr("Undefined argument: %s \n", argv[optind++]);
		}
		printfErr("Shutting down\n");
		exit(EXIT_FAILURE);
	}

	printfLog("Starting protocol\n");	
	
	if (meta.post) testAll();
	
	setup();

	handler();

	clean();

	exit(EXIT_SUCCESS);
}

void
setup()
{
	int rc;

	self.is_master = isMaster();

	if (pthread_create(&(meta.WF_listener_t), NULL, WF_listener, NULL)) {
		printfErr("Error: Unable to create thread, %d\n", rc);
		exit(-1);
	}

	if (pthread_create(&(meta.WF_dispatcher_t), NULL, WF_dispatcher, NULL)) {
		printfErr("Error: Unable to create thread, %d\n", rc);
		exit(-1);
	}
	
	if (pthread_create(&(meta.WS_listener_t), NULL, WS_listener, NULL)) {
		printfErr("Error: Unable to create thread, %d\n", rc);
		exit(-1);
	}

	if (pthread_create(&(meta.HW_dispatcher_t), NULL, HW_dispatcher, NULL)) {
		printfErr("Error: Unable to create thread, %d\n", rc);
		exit(-1);
	}
}

void
handler()
{
	void* message;

	while (1) {
		message = getMessage();
		if (message== NULL) continue;
		switch (((byte*)message)[0]) {
			case SD:
				handleSD((SD_p*)message);
				break;
			case PB:
				handlePB((PB_p*)message);
				break;
			case PR:
				handlePR((PR_p*)message);
				break;
			default:
				printfLog("Unrecognized message type %d\n", ((byte*)message)[0]);
		}
		free(message);
	}
}

void
clean()
{
	if (meta.log) fclose(meta.log);
}


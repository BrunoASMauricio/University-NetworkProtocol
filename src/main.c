#include "main.h"

int
/*
 * Main function
 * Parses the command line arguments (sets verbosity and debugging)
 * Starts the main protocol loop
 */
main(int argc, char **argv)
{
	int c;

	meta.debug = false;
	meta.post= false;
	meta.log = NULL;
	self.is_master = UNSET;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"log",		no_argument,		0, 'l'},
			{"post",	no_argument,		0, 'p'},
			{"debug",	no_argument,		0, 'd'},
			{0,			0,					0,  0 }
		};

		c = getopt_long(argc, argv, "lpdr:", long_options, &option_index);

		if (c == -1)	break;

		switch (c) {
			case 'r':
				if (optarg[0] == 'M') {
					self.is_master = true;
					printf("Forcing node to master\n");
				} else if (optarg[0] == 'S') {
					self.is_master = false;
					printf("Forcing node to slave\n");
				} else {
					fatalErr("Node can only be master (M) or slave (S)\n");
				}
				break;

			case 'l':
				if ((meta.log = fopen("./log", "a")) == NULL) {
					fatalErr("Could not open log. Errno set to: %d\n", errno);
				}
				break;

			case 'd':
				meta.debug = true;
				break;

			case 'p':
				meta.post= true;
				break;

			case '?':
				fatalErr("Undefined argument\n");
				break;

			default:
				printf("?? getopt returned character code 0%o ??\n", c);
		}
	}

	printf("Starting protocol\n");	
	
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

	self.outbound_q = newQueue();
	self.inbound_q = newQueue();

	if (rc = pthread_create(&(meta.WF_listener_t), NULL, WF_listener, NULL)) {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}

	if (rc = pthread_create(&(meta.WF_dispatcher_t), NULL, WF_dispatcher, NULL)) {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}
	
	if (rc = pthread_create(&(meta.WS_listener_t), NULL, WS_listener, NULL)) {
		fatalErr("Error: Unable to create thread, %d\n", rc);
	}

	if (rc = pthread_create(&(meta.HW_dispatcher_t), NULL, HW_dispatcher, NULL)) {
		fatalErr("Error: Unable to create thread, %d\n", rc);
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
				printf("Unrecognized message type %d\n", ((byte*)message)[0]);
		}
		free(message);
	}
}

void
clean()
{
	if (meta.log) fclose(meta.log);
}


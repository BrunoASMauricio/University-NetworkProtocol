NP Protocol V2

The specification for this version is present in wiki/Versions/V1

The implementation code is in ./src

It is advised to always run with -d

Code parameters:

    -r [S|M] : force node to be either a master or a slave

    -d/--debug : Debugging is enabled (every error is outputted)

    -p/--post : Enable Power On Self Testing (launches "testAll()" which performs all tests all data structure manipulation functions).
	
	-s/--Sim : Enable timestamp output (for simulator purposes)

	-H/--HW : HW interface port

	-W/--WS : WS interface port

	-T/--WF_TX : WF TX interface port

	-R/--WF_RX : WF RX interface port

	-I/--IP : Force IP (requires last IP byte, forced IPs always have 0x82 as the first byte)
	

Usage example:
    ./NP -dp -r S --HW 1234 --WS 1235 --WF_TX 1236 --WF_RX 1237 --IP 2

Compiling:
	Just type "make" in the same directory, the makefile resides in.

Code Structure:
	main	Parse command line arguments;
			Handle node bootstrap;
			Launch RX/TX threads;
			clean on exit.

	data	Global data structures, as well as the main functions that interact with said structures

	protocol Handles most protocol specific logic

	TX		 Handle outwards communications

	RX		 Handle inwards communications

	RoutingTable     Handle routing table functionalities

	Udp              Handle udp sockets 

	debug	         Provides debugging and logging
			 Also contains self testing routines

Data structures:
	The two main data structures are "meta_data" and "node";

	meta_data:
		Contains data to control the program itself (threads, logging, debugging, etc)
		Should only have 1 instace, called "Meta", which is global (instanciated in data.h)
		Interactions MUST BE THREAD SAFE!

	node:
		Contains protocol relevant data (queues, protocol control information, etc)
		Should only have 1 instace, called "Self", which is global (instanciated in data.h)
		Interactions MUST BE THREAD SAFE!
	
	queue:
		Implementation of an ordered FIFO.
		All functions implemented MUST BE THREAD SAFE!

	SD_p, PB_p and PR_p:
		Packet holding data structure

	sample:
		Holds the information of a single sample
	

Considerations for real runs:
    Deactivate sleep/hibernation
    
    Launch with high priority
        https://www.tecmint.com/set-linux-process-priority-using-nice-and-renice-commands/
        nice -20 ./NP






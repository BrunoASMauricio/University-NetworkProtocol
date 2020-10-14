NP Protocol V1

The specification for this version is present in wiki/Versions/V1

The implementation code is in ./src

It is advised to always run with -d

Code parameters:

    -r [S|M] : force node to be either a master or a slave

    -l : Enable logging (prints are stored in the "./log" file)

    -d : Debugging is enabled (every error is outputted)

    -p : Enable Power On Self Testing (launches "testAll()" which performs all tests all data structure manipulation functions).

Usage example:
    ./NP -ldp -r S


Code Structure:
	main	Parse command line arguments;
			Handle node bootstrap;
			Launch RX/TX threads;
			clean on exit.

	data	Global data structures

	protocol Handles most protocol specific logic

	TX		Handle outwards communications

	RX		Handle inwards communications

	debug	Provides debugging and logging
			Also contains self testing routines

Compiling:
	Just type "make" in the same directory, the makefile resides in.







NP Protocol V1

The specification for this version is present in wiki/Versions/V1

The implementation code is in ./src

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


	







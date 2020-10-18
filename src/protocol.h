#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
 * Performs a network test
 * Returns true if this node is the master
 * (has an internet connection)
 * false otherwise
 */
bool
isMaster();


void
handleSD(void* message);

void
handlePB(void* message);

void
handlePR(void* message);

#endif

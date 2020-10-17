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
handleSD(SD_p* message);

void
handlePB(PB_p* message);

void
handlePR(PR_p* message);

#endif

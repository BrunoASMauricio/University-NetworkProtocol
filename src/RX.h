#ifndef RX_H
#define RX_H

void*
/*
 * Interface with the WF team (RX)
 * Continuously listens for new packets
 * Discards packets unless:
 * * The message type is a PB;
 * * The next hop IP is the broadcast IP (all 1s);
 * * Its' IP is the Next Hop IP in the message.
 * * Its' IP is the Source IP and the Next Hop IP isn't the one sent
 * dummy:
 * * A dummy variable, contains NULL
 *
 */
WF_listener(void* dummy);

void*
/*
 * Interface with the WS team (RX)
 */
WS_listener(void* dummy);



void*
/*
 * Blocks until a new packet is available in the inbound queue
 * Should release the CPU while blocking
 */
getMessage();

#endif

#ifndef MAIN_H
#define MAIN_H

#include "imports.h"


/*
 * Performs node setup
 */
void
setup();

/*
 * Continuously handles the received packets
 */
void
handler(void* _Message);


/*
 * Clean the data structures
 * Save whatever data needs saving
 */
void
clean();

/*
 * Signal handler wrapper for clean
 */
void
clean(int signo);

#include "./utils/udp.c"
#include "./utils/debug.c"
#include "./utils/macros.h"
#include "./utils/queue.c"
// node.c requires this
#include "./utils/list.h"

#include "./data_structures/node.c"
#include "./utils/list.c"

#include "./data_structures/routing_table.c"
#include "./data_structures/pbid_table.c"
#include "./data_structures/message_build.c"
#include "./data_structures/time_table.c"

#include "./TX/network.c"
#include "./TX/application.c"
#include "./TX/routing.c"
#include "./TX/time_table.c"
#include "./TX/retransmission.c"
#include "./TX/WF.c"
#include "./TX/HW.c"

#include "./utils/equations.c"

#include "./RX/network.c"
#include "./RX/routing.c"
#include "./RX/application.c"
#include "./RX/time_table.c"
#include "./RX/WF.c"
#include "./RX/WS.c"

/*
#include "./tests/network.c"
#include "./tests/queue.c"
#include "./tests/pbid_table.c"
#include "./tests/performance.c"
#include "./tests/time_table.c"
#include "./tests/routing_table.c"
#include "./tests/routing.c"
#include "./tests/list.c"
*/
#include "./tests/test.c"
#endif

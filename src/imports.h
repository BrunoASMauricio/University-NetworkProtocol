#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <ifaddrs.h>
#include <dirent.h>
#include <netdb.h>
#include <ctype.h>


#include "./utils/list.h"
#include "./utils/macros.h"
#include "./utils/messages.h"
#include "./utils/queue.h"
#include "./utils/udp.h"
#include "./utils/debug.h"


#include "./data_structures/time_table.h"
#include "./data_structures/pbid_table.h"
#include "./data_structures/node.h"
#include "./data_structures/message_build.h"
#include "./data_structures/routing_table.h"
#include "./data_structures/retransmission.h"

#include "./TX/network.h"
#include "./TX/application.h"
#include "./TX/routing.h"
#include "./TX/time_table.h"
#include "./TX/retransmission.h"
#include "./TX/WF.h"
#include "./TX/HW.h"

#include "./RX/network.h"
#include "./RX/routing.h"
#include "./RX/application.h"
#include "./RX/time_table.h"
#include "./RX/WF.h"
#include "./RX/WS.h"


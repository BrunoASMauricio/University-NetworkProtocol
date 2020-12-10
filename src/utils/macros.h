
#define SETBIT(bit,byte) byte |= (1 << bit)
#define CLEARBIT(bit,byte) byte &= ~(1 << bit)
#define CHECKBIT(bit,byte) (byte) & (1 << bit)

//#define range(min, max) (rand()/RAND_MAX) * (max - min + 1) + min
#define range(min, max) (unsigned long int)(((double)rand()/(double)RAND_MAX)* (double)(max-min+1) + (double)min)

#define printf printfLog

typedef uint8_t byte;
typedef uint16_t pbid;

void
/*
 * print a message, and dump a binary
 */
dumpBin(char* buf, int size, const char *fmt,...);

void
/*
 * Print the message type
 * in the buffer and dump buffer
 */
printMessage(void* buff, int size);

void
/*
 * Normal output
 */
printfLog(const char *fmt, ...);

void
/*
 * Try to print to a file
 * Print to stderr
 * End program with EXIT_FAILURE
 */
fatalErr(const char *fmt, ...);


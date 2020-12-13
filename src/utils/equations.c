#include "equations.h"


unsigned short updateDistance (float Quality, int NumberOfSubSlaves, int MaxThroughput)
{
    return (float)((31- Quality)/Quality+100);

}
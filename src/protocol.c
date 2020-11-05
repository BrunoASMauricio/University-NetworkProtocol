#include "protocol.h"


void startRetransmission(retransmitable message_type)
{
	timespec Res;
	unsigned long int Act;
	pthread_mutex_lock(&(Self.Rt.Lock));

	SETBIT(message_type, Self.Rt.Retransmitables);
	
	clock_gettime(CLOCK_REALTIME, &Res);
	Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;
	switch(message_type){
		case rTB:
			Self.Rt.Time_TB = Act + RETRANSMISSION_DELAY_TB;
			break;
		case rPR:
			Self.Rt.Time_PR = Act + RETRANSMISSION_DELAY_PR;
			break;
		case rNE:
			Self.Rt.Time_NE = Act + RETRANSMISSION_DELAY_NE;
			break;
		case rNER:
			Self.Rt.Time_NER = Act + RETRANSMISSION_DELAY_NER;
			break;
	}
	
	pthread_mutex_unlock(&(Self.Rt.Lock));
}

void stopRetransmission(retransmitable message_type)
{
	pthread_mutex_lock(&(Self.Rt.Lock));
	CLEARBIT(message_type, Self.Rt.Retransmitables);
	pthread_mutex_unlock(&(Self.Rt.Lock));
}

void* retransmit(void* dummy)
{
	unsigned long int earliest;
	unsigned long int Act;
	timespec Res;
	printf("Retransmission thread on\n");
	while(1)
	{
		earliest = 0;
		clock_gettime(CLOCK_REALTIME, &Res);
		Act = Res.tv_sec * (int64_t)1000000000UL + Res.tv_nsec;

		pthread_mutex_lock(&(Self.Rt.Lock));
		
		if(Self.Rt.Retransmitables & rTB)
		{
			if(Act > Self.Rt.Time_TB)
			{
				// TB_TX();
				Self.Rt.Time_TB += RETRANSMISSION_DELAY_TB;
			}
			else if(Self.Rt.Time_TB > earliest)
			{
				earliest = Self.Rt.Time_TB;
			}
		}
		if(Self.Rt.Retransmitables & rPR)
		{
			if(Act > Self.Rt.Time_PR)
			{
				// PR_TX();
				Self.Rt.Time_PR += RETRANSMISSION_DELAY_PR;
			}
			else if(Self.Rt.Time_PR > earliest)
			{
				earliest = Self.Rt.Time_PR;
			}
		}
		if(Self.Rt.Retransmitables & rNE)
		{
			if(Act > Self.Rt.Time_NE)
			{
				// NE_TX();
				Self.Rt.Time_NE += RETRANSMISSION_DELAY_NE;
			}
			else if(Self.Rt.Time_NE > earliest)
			{
				earliest = Self.Rt.Time_NE;
			}
		}
		if(Self.Rt.Retransmitables & rNER)
		{
			if(Act > Self.Rt.Time_NER)
			{
				// NER_TX();
				Self.Rt.Time_NER += RETRANSMISSION_DELAY_NER;
			}
			else if(Self.Rt.Time_NER > earliest)
			{
				earliest = Self.Rt.Time_NER;
			}
		}

		pthread_mutex_unlock(&(Self.Rt.Lock));
		
		if(earliest)
		{
			usleep(earliest/1000);
		}
		else
		{
			usleep(DEFAULT_RETRANSMIT_CHECK);
		}
	}
	return NULL;
}

void
insertSubSlave(byte IP[2])
{
	insertIPList(Self.SubSlaves, IP);
}

bool
getSubSlave(byte IP[2])
{
	return getIPFromList(Self.SubSlaves, IP);
}

void
removeSubSlave(byte IP[2])
{
	removeIPList(Self.SubSlaves, IP);
}

void
insertOutsideSlave(byte IP[2])
{
	insertIPList(Self.OutsideSlaves, IP);
}

bool
getOutsideSlave(byte IP[2])
{
	return getIPFromList(Self.OutsideSlaves, IP);
}

void
removeOutsideSlave(byte IP[2])
{
	removeIPList(Self.OutsideSlaves, IP);
}

void
setMaster()
{
	const char *Hostname;
	struct hostent *HostInfo;
	
	if(Self.IsMaster != UNSET)
	{
		return;
	}
	
	Hostname = "google.com";
	HostInfo = gethostbyname (Hostname);

	if(HostInfo == NULL)
	{
		Self.IsMaster=false;
	}
	else
	{
		Self.IsMaster=true;
	}
	
}

int
getPacketSize(void* buf)
{
	byte type, version;
	int helper;
	version = (((byte*)buf)[0] >> 4) & 0x0f;

	if(version != PROTOCOL_VERSION)
	{
		printfErr("Wrong version. Got %d, was expecting %d\n", version, PROTOCOL_VERSION);
		return -1;
	}

   	type = ((char*)buf)[0] & 0x0f;
	
	switch(type)
	{
		case SD:
			helper = ((char*)buf)[6]*SAMPLE_SIZE;
			if(8*(helper/8) != helper)
			{
				helper = helper/8 +1;
			}
			else
			{
				helper /= 8;
			}
			return Packet_Sizes[SD] + helper;
		case TB:
            //NOTE(GoncaloXavier): As per clarification on MR !9 - WF:
            //((short*)buf)[8]*2*8->Table size (2 bytes) nº of IP's * IP size
            //((short*)buf)[8] -> bitmap size
			helper = ((short*)buf)[8];
			if(8*(helper/8) != helper)
			{
				helper = helper/8 +1;
			}
			else
			{
				helper /= 8;
			}
			return Packet_Sizes[TB] + ((short*)buf)[8]*2 + helper;
		default:
			if(type > sizeof(Packet_Sizes) -1)
			{
				printfErr("Unrecognized Message type %d\n", type);
				return -1;
			}
			return Packet_Sizes[type];
	}
}

void
handleSD(void* message)
{

}

void
handlePB(void* message)
{

}

void
handlePR(void* message)
{

}
unsigned char* ConverMacAddressStringIntoByte(const char *pszMACAddress, unsigned char* pbyAddress)
{
    const char cSep = ':';
	for(int iConunter = 0; iConunter < 6; ++iConunter)
	{
		unsigned int iNumber = 0;
		char ch;

		//Convert letter into lower case.
		ch = tolower (*pszMACAddress++);

		if((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
		{
			return NULL;
		}

		//Convert into number.
		//       a. If character is digit then ch - '0'
		//	b. else (ch - 'a' + 10) it is done 
		//	because addition of 10 takes correct value.
		iNumber = isdigit (ch) ? (ch - '0') : (ch - 'a' + 10);
		ch = tolower (*pszMACAddress);

		if((iConunter < 5 && ch != cSep) ||
			(iConunter == 5 && ch != '\0' && !isspace (ch)))
		{
			++pszMACAddress;

			if((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
			{
				return NULL;
			}

			iNumber <<= 4;
			iNumber += isdigit (ch) ? (ch - '0') : (ch - 'a' + 10);
			ch = *pszMACAddress;

			if(iConunter < 5 && ch != cSep)
			{
				return NULL;
			}
		}
		/* Store result.  */
		pbyAddress[iConunter] = (unsigned char) iNumber;
		/* Skip cSep.  */
		++pszMACAddress;
	}
	return pbyAddress;
}

byte*
getIP()
{

	if(Self.IP[0] != 0 && Self.IP[1] != 0){
		return Self.IP;
	}

    byte* ip = (byte*)malloc(sizeof(byte)*2);
    FILE* fp;

    char mac_add_string [17];
    unsigned char mac_add_byte[6];

    //NOTE(Rita): Debian dependency due to specific interface name
    fp = fopen("/sys/class/net/enp0s5/address","r"); 

    if(fp == NULL)
    {
       printf("Could not open mac address file. Errno set to: %d\n", errno);
       return NULL;
    }
	
    fscanf(fp,"%s",mac_add_string );

    if( ConverMacAddressStringIntoByte(mac_add_string, mac_add_byte)== NULL)
    {
        printf("Couldn't convert to mac string to mac bytes\n");
        return NULL;
    }

    ip[0] = mac_add_byte[4];
    ip[1] = mac_add_byte[5];

    return ip;
}

/*
typedef struct{
	pthread_mutex_t Lock;
	byte PBID[2];
	IPList* IPs;
	void* Bitmap;
	byte Bitmap_size;
	long int Sync_timestamp;
	short Validity_delay;
} timetable_msg;
*/

bool getBitmapValue(short* IP, void* bitmap, int size, void* IPs)
{
	int place = -1;
	byte* local_byte;

	for(int i = 0; i < size; i++)
	{
		if(((short*)IPs)[i] == IP[0])
		{
			place = i;
			break;
		}
	}

	if(place == -1)
	{
		printfErr("Could not find IP in bitmap");
		dumpBin((char*)IPs, size, "IPs: ");
		return false;
	}
	local_byte = (byte*)bitmap + (place/8);
	place = place - 8 * (place/8);
	return (0x80 >> place) & local_byte[0];
}

timetable* newTimeTable()
{
	timetable* tm = (timetable*)malloc(sizeof(timetable));

	if(pthread_mutex_init(&(tm->Lock), NULL) != 0)
    {
        fatalErr("mutex init failed for new IP list lock\n");
    }
	return tm;
}

void delTimeTable(timetable* tm)
{
	pthread_mutex_destroy(&(tm->Lock));
	free(tm);
}

void* generateTB()
{
	timespec res;
	void* buff;
	byte rest;
	byte* IP;
	int ip_amm;
	short* IPHolder;
	int size;

	pthread_mutex_lock(&(Self.SubSlaves->Lock));
	printf("Building TB\n");
	ip_amm = Self.SubSlaves->L->Size;
	size = (ip_amm+1)*(2*8+1);

	if(8*(size/8) != size)
	{
		size = size/8 + 1;
	}
	else
	{
		size /= 8;
	}
	size += Packet_Sizes[TB];

	buff = (void*)malloc(size);
	printf("Allocating %d bytes for TB\n", size);
	((byte*)buff)[0] = (0xf0 & (PROTOCOL_VERSION<<4)) | TB;
	((byte*)buff)[1] = Self.IP[0];
	((byte*)buff)[2] = Self.IP[1];
	((byte*)buff)[3] = Self.TB_PBID[0];
	((byte*)buff)[4] = Self.TB_PBID[1];
	clock_gettime(CLOCK_REALTIME, &res);
	((unsigned long int*)((byte*)buff+5))[0] = res.tv_sec * (int64_t)1000000000UL + res.tv_nsec;
	printf(" OUT %lu\n", ((unsigned long int*)((byte*)buff+5))[0]);
	((short*)(((byte*)buff+13)))[0] = DEFAULT_VALIDITY_DELAY;
	(((byte*)buff+15))[0] = DEFAULT_TIMESLOT_SIZE;
	((short*)(((byte*)buff+16)))[0] = ip_amm+1;	// Account for self
	printf("Sub-Slave IP Ammount %d\n", ip_amm);

	for(int i = 0; i < ip_amm; i++)
	{
		pthread_mutex_unlock(&(Self.SubSlaves->Lock));
		IPHolder = getIPFromList(Self.SubSlaves, i);
		pthread_mutex_lock(&(Self.SubSlaves->Lock));
		((short*)(((byte*)buff+18)))[i] = IPHolder[0];
		printf("Adding IP: %d %d at %d\n", ((byte*)IPHolder)[0],  ((byte*)IPHolder)[1], 18+2*i);
	}
	printf("Adding self IP\n");
	((short*)(((byte*)buff+18)))[ip_amm] = ((short*)Self.IP)[0];
	ip_amm += 1;

	printf("Adding bitmap at %d\n", 18+ip_amm*2);
	
	for(int i = 0; i < ip_amm/8; i++)
	{
		printf("Cool\n");
		((byte*)buff)[18+ip_amm*2+i] = 0xff;
	}
	rest = ip_amm - 8*(ip_amm/8);
	
	if(rest)
	{
		// No point in "cutting" the last bits, because the bitmap must
		// already cut them
		printf("Cool 2\n");
		((byte*)buff)[18+ip_amm*2+(ip_amm/8)] = (0xff<<(8-rest));
	}
	
	pthread_mutex_unlock(&(Self.SubSlaves->Lock));
	
	return buff;
}



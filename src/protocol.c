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
	int type, version;
	version = ((char*)buf)[0] & 0xf0 >> 4;

	if(version != PROTOCOL_VERSION)
	{
		printfErr("Wrong version. Got %d, was expecting %d\n", version, PROTOCOL_VERSION);
		return -1;
	}

   	type = ((char*)buf)[0] & 0x0f;
	
	switch(type)
	{
		case SD:
			return Packet_Sizes[SD] + ((char*)buf)[6]*SAMPLE_SIZE;
		case TB:
            //NOTE(GoncaloXavier): As per clarification on MR !9 - WF:
            //((short*)buf)[8]*2*8->Table size (2 bytes) nº of IP's * IP size
            //((short*)buf)[8] -> bitmap size
			return Packet_Sizes[TB] + ((short*)buf)[8]*2*8 + ((short*)buf)[8];
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

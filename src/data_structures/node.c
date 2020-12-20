#include "node.h"

void
setMaster()
{
	const char *Hostname;
	struct hostent *HostInfo;
	
	if(Self.IsMaster != UNSET)
	{
		return;
	}
	
	socket_s* sockfd = newSocket(Meta.HW_port);
	
  
	if ((sockfd->s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 ||
	inet_aton("127.0.0.1" , &(sockfd->sockaddr.sin_addr)) == 0 )
	{
		Self.IsMaster=false;
	}
	else
	{
		Self.IsMaster=true;
	}
	
	closeSocket(sockfd);
}

int
getPacketSize(void* buf)
{
	byte type, version;
	int helper;
	version = (((byte*)buf)[0] >> 4) & 0x0f;

	if(version != PROTOCOL_VERSION)
	{
		printf("Wrong version. Got %d, was expecting %d\n", version, PROTOCOL_VERSION);
		return -1;
	}

   	type = ((char*)buf)[0] & 0x0f;
	
	switch(type)
	{
		case SD:
			helper = ((char*)buf)[6] & 0xff;
			return Packet_Sizes[SD] + helper;
		case TB:
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
				printf("Unrecognized Message type %d\n", type);
				return -1;
			}
			return Packet_Sizes[type];
	}
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

	if(Self.IP[0] != 0 && Self.IP[1] != 0){ //so if somebody uses getIp instead of Self.Ip
		return Self.IP;
	}

    byte* Ip = (byte*)malloc(sizeof(byte)*2);
    FILE* fp;
	DIR *pDir;
	struct dirent *pDirent;
	char mac_add_string [17];
    unsigned char mac_add_byte[6];
	char buf[300];
     
    pDir = opendir ("/sys/class/net/");
    if (pDir == NULL) {
    	fatalErr("Cannot open directory '/sys/class/net/'\n");

    }

    while ((pDirent = readdir(pDir)) != NULL) {
        
            if(pDirent->d_name[0] == 'e' && (pDirent->d_name[1] == 'n' || pDirent->d_name[1] == 't')){
                        
                    snprintf(buf, sizeof buf, "%s%s%s", "/sys/class/net/", pDirent->d_name, "/address");
                        fp = fopen(buf,"r"); 
                        fscanf(fp,"%s",mac_add_string );
			}
        }

    
    closedir(pDir);


    if( ConverMacAddressStringIntoByte(mac_add_string, mac_add_byte)== NULL)
    {
        printf("Couldn't convert to mac string to mac bytes\n");
        return NULL;
    }

    Ip[0] = mac_add_byte[4];
    Ip[1] = mac_add_byte[5];

    return Ip;
}
pbid 
getNewPBID()
{
    return ++(Self.PBID);
}


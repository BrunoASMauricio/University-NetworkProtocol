#include "protocol.h"


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
getPacketSize(void* buf){
	int type, version;
	version = ((char*)buf)[0] & 0xf0 >> 4;
	if(version != PROTOCOL_VERSION)
	{
		printfErr("Wrong version. Got %d, was expecting %d\n", version, PROTOCOL_VERSION);
		return -1;
	}
   	type = ((char*)buf)[0] & 0x0f;
	switch(type){
		case SD:
			return Packet_Sizes[SD] + ((char*)buf)[7]*SAMPLE_SIZE;
		case TB:
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

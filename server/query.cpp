/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

    Version: $Id: query.cpp,v 1.16 2006/05/08 13:28:46 kyeman Exp $

*/

#include "main.h"

bool bRconSocketReply = false;

void RconSocketReply(char* szMessage)
{
	if (bRconSocketReply) {
		size_t nLength = strlen(szMessage);
	}
}

int ProcessQueryPacket(unsigned int binaryAddress, unsigned short port, char* data, int length, SOCKET s)
{
	// Expecting atleast 10 bytes long data, starting first 4 bytes with "SAMP"
	if (length >= 11 && *(unsigned int*)data == 0x504D4153) {

		// Tell the user someone sent a request, if "logqueries" enabled
		if (pConsole->GetBoolVariable("logqueries")) {
			in_addr in;
			in.s_addr = binaryAddress;
			logprintf("[query:%c] from %s:%d", data[10], inet_ntoa(in), port);
		}

		// Data was in fact query request 
		return 1;
	}
	return 0;
}

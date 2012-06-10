/***
*
*	Copyright © 2012, AGHL.RU, All rights reserved.
*
*	Purpose: Network communications.
*
****/

#include "net.h"
#include <windows.h>
#include <winsock.h>


bool g_bInitialised = false;
bool g_bFailedInitialization = false;


///
/// Winsock initialization.
///
void WinsockInit()
{
	g_bInitialised = true;

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(1, 1);
	if (WSAStartup(wVersionRequested, &wsaData))
	{
		g_bFailedInitialization = true;
		return;
	}

	/* Confirm that the Windows Sockets DLL supports 1.1
	 * Note that if the DLL supports versions greater
	 * than 1.1 in addition to 1.1, it will still return
	 * 1.1 in wVersion since that is the version we
	 * requested. */
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		g_bFailedInitialization = true;
		return;
	}
}

int NetSendReceiveUdp(const char *addr, int port, const char *sendbuf, int len, char *recvbuf, int size)
{
	unsigned long ulAddr = inet_addr(addr);
	return NetSendReceiveUdp(ulAddr, htons((unsigned short)port), sendbuf, len, recvbuf, size);
}

int NetSendReceiveUdp(unsigned long sin_addr, int sin_port, const char *sendbuf, int len, char *recvbuf, int size)
{
	if (!g_bInitialised)
		WinsockInit();
	if (g_bFailedInitialization)
		return -1;

	// Create a socket for sending data
	SOCKET SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in RecvAddr, fromaddr;
	memset(&RecvAddr, 0, sizeof(struct sockaddr_in));
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_addr.s_addr = sin_addr;
	RecvAddr.sin_port = sin_port;

	sendto(SendSocket, sendbuf, len, 0, (struct sockaddr *) &RecvAddr, sizeof(struct sockaddr_in));

	// Receive response
	int res;
	unsigned long nonzero = 1;
	ioctlsocket(SendSocket, FIONBIO, &nonzero);
	SOCKET maxfd;
	fd_set rfd;
	fd_set efd;
	struct timeval tv;
	int i = 0;
	while (i < 3)
	{
		// Do select on socket
		FD_ZERO(&rfd);
		FD_ZERO(&efd);
		FD_SET(SendSocket, &rfd);
		FD_SET(SendSocket, &efd);
		maxfd = SendSocket;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		res = select(maxfd + 1, &rfd, NULL, &efd, &tv);
		// Check for error on socket
		if (res == SOCKET_ERROR || FD_ISSET(SendSocket, &efd))
		{
			closesocket(SendSocket);
			return -1;
		}
		int fromaddrlen = sizeof(struct sockaddr_in);
		res = recvfrom(SendSocket, recvbuf, size, 0, (struct sockaddr *) &fromaddr, &fromaddrlen);
		if (res == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
		{
			closesocket(SendSocket);
			return -1;
		}
		if (RecvAddr.sin_addr.s_addr != fromaddr.sin_addr.s_addr || RecvAddr.sin_port != fromaddr.sin_port)
		{
			continue;
		}
		if (res >= 0)
		{
			closesocket(SendSocket);
			return res;
		}
		i++;
	}
	closesocket(SendSocket);
	return -1;
}

char *NetGetRuleValueFromBuffer(const char *buffer, int len, const char *cvar)
{
	// Response header
	if (*((unsigned int*)buffer) != 0xFFFFFFFF) return NULL;
	// Search for a cvar
	char *current = (char *)buffer + 4;
	char *end = (char *)buffer + len - strlen(cvar);
	while (current < end)
	{
		char *pcvar = (char *)cvar;
		while(*current == *pcvar && *pcvar != 0)
		{
			current++;
			pcvar++;
		}
		if (*pcvar == 0)
		{
			// Found
			return current + 1;
		}
		current++;
	}
	return NULL;
}

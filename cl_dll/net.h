//========= Copyright © 2012, AGHL.RU, All rights reserved. ============
//
// Purpose: Network communications.
//
//=============================================================================

#if !defined( NETH )
#define NETH
#pragma once

#include <winsock.h>

int NetSendReceiveUdp(const char *addr, int port, const char *sendbuf, int len, char *recvbuf, int size);
int NetSendReceiveUdp(unsigned long sin_addr, int sin_port, const char *sendbuf, int len, char *recvbuf, int size);
int NetSendUdp(unsigned long sin_addr, int sin_port, const char *sendbuf, int len, SOCKET *socket_out);
int NetReceiveUdp(unsigned long sin_addr, int sin_port, char *recvbuf, int size, SOCKET socket_in);
void NetClearSocket(SOCKET s);
void NetCloseSocket(SOCKET s);

char *NetGetRuleValueFromBuffer(const char *buffer, int len, const char *cvar);

#endif

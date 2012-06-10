//========= Copyright © 2012, AGHL.RU, All rights reserved. ============
//
// Purpose: Network communications.
//
//=============================================================================

#if !defined( NETH )
#define NETH
#pragma once

int NetSendReceiveUdp(const char *addr, int port, const char *sendbuf, int len, char *recvbuf, int size);
int NetSendReceiveUdp(unsigned long sin_addr, int sin_port, const char *sendbuf, int len, char *recvbuf, int size);

char *NetGetRuleValueFromBuffer(const char *buffer, int len, const char *cvar);

#endif

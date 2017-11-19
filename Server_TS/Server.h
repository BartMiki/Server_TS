#pragma once

#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Protocol.h"

#pragma comment(lib,"ws2_32.lib") 

class Server
{
public:
	// --- methods ---
	Server(int PORT, bool broadcastLocally = false);
	bool ListenForNewConnections();
private:
	// --- data fields ---
	static const int connectionsSize = 100;
	SOCKET connections[connectionsSize]; //Array to hold connections
	int TotalConnections = 0;
	WSAData wsaData;
	WORD DllVersion;
	SOCKADDR_IN addr; //Address to bind listening socket to
	SOCKET sListen;
	int addrLen;

	// --- methods send ---
	bool sendProtocol(int ID, Protocol * protocol);

	// --- methods recv ---
	Protocol * recvProtocol(int ID);

	// --- methods other ---
	bool processProtocol(int ID, Protocol * protocol);
	static void ClientHandlerThread(int ID);
};

static Server * serverPtr;


#pragma once

#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Protocol.h"
#include "Client.h"
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

#pragma comment(lib,"ws2_32.lib") 

class Server
{
public:
	// --- methods ---
	Server(int PORT, bool broadcastLocally = false);
	bool ListenForNewConnections();
private:
	// --- data fields ---
	static vector<Client*> clients;
	WSAData wsaData;
	WORD DllVersion;
	SOCKADDR_IN addr; //Address to bind listening socket to
	SOCKET sListen;
	int addrLen;

	// --- methods send ---
	bool sendProtocol(Client * client, Protocol * protocol);

	// --- methods recv ---
	Protocol * recvProtocol(Client * client);

	// --- methods other ---
	bool processProtocol(Client * client, Protocol * protocol);
	static void ClientHandlerThread(Client * client);
};

static Server * serverPtr;


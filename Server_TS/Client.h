#pragma once
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib") 

class Client
{
public:
	u_int64 sessionID;
	SOCKET clientSocket;
	Client(SOCKET &sock, u_int64 id);
	~Client();
};


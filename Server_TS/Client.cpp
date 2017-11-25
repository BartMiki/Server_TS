#include "stdafx.h"
#include "Client.h"


Client::Client(SOCKET &sock, u_int64 id) : clientSocket(sock), sessionID(id){}

Client::~Client()
{}

// Server_TS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Server.h"

int main()
{
	Server myServer(5555);
	for (int i = 0; i < 100; i++)
	{
		myServer.ListenForNewConnections();
	}
	getchar();
    return 0;
}


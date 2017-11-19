#include "stdafx.h"
#include "Server.h"


Server::Server(int PORT, bool broadcastLocally)
{
	DllVersion = MAKEWORD(2, 2);

	//if WSAStartup return anything other than 0, that means an error has occured
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(NULL, "WinSock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	// --- Address setup ---
	addrLen = sizeof(addr); //Address lenght
	if (broadcastLocally) //Server open to public
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else //Server only for our router
		inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));//Brodacast locally
	addr.sin_port = htons(PORT); //Port ("htons" means "Host TO Network Short")
	addr.sin_family = AF_INET; //IPv4 Socket

	// --- Binding address to listening socket ---
	sListen = socket(AF_INET, SOCK_STREAM, NULL); //Socket for listening new connections
	if (bind(sListen, (SOCKADDR*)&addr, addrLen) == SOCKET_ERROR) //Bind address to the socket
	{
		std::string error = "Failed to bind address to listetnin socket: " + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, error.c_str(), "Error", MB_OK | MB_ICONERROR);
	}
	if (listen(sListen, SOMAXCONN) == SOCKET_ERROR) //sListen socket is listening
	{
		std::string error = "Failed to listen on listetnin socket: " + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, error.c_str(), "Error", MB_OK | MB_ICONERROR);
	}
	serverPtr = this;
}

bool Server::ListenForNewConnections()
{
	SOCKET newConnection; //Hold's client connection
	newConnection = accept(sListen, (SOCKADDR*)&addr, &addrLen); //Accept a new connection
	if (newConnection == 0) //If accepting the client connection failed
	{
		std::cout << "Failed to accept client's connection." << std::endl;
		return false;
	}
	else //If client connection accepted
	{
		std::cout << "Client connected!" << std::endl;
		connections[TotalConnections] = newConnection;
		//Create Thread to handle this client. Value i is the index to connections array
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)TotalConnections, NULL, NULL);
		std::string message = "Welcome! Happy to see you here!.\0"; //Buffer with messege
		//SendString(TotalConnections, message, Packet::MsgServer);
		TotalConnections++;
		return true;
	}
}

bool Server::processProtocol(int ID, Protocol * protocol)
{
	if (protocol == nullptr)
		return false;

	PacketHeader packetHeader = protocol->getPacketHeader();
	u_int64 dataSize = protocol->getDataSize();
	string data = "";
	u_int64 SesionID = protocol->getID();
	if (dataSize > 0)
	{
		protocol->getData();
	}
	switch (packetHeader)
	{
	case PacketHeader::ServerRequestID:
	{
		sendProtocol(ID, new Protocol(PacketHeader::ServerSendID, 0, "", ID));
		break;
	}
	case PacketHeader::Quit:
	{
		break;
	}
	case PacketHeader::Message:
	{
		for (int i = 0; i < TotalConnections; i++)
		{
			if (i == ID) {
				continue;
			}
			if (sendProtocol(i, protocol)) {
				cout << "Przeslano wiadomosc do klienta o ID = " << ID << "\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem wiadomosci do uzytkownika o ID = " << ID << "\n";
			}
		}
		break;
	}
	case PacketHeader::Bye:
	{
		for (int i = 0; i < TotalConnections; i++)
		{
			if (i == ID) {
				continue;
			}
			if (sendProtocol(i, protocol)) {
				cout << "Przeslano Invite do klienta o ID = " << ID << "\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem Invite do uzytkownika o ID = " << ID << "\n";
			}
		}
		break;
	}
	case PacketHeader::Invite:
	{
		cout << "Klient o ID = " << ID << " wyslal wiadomosc Invite\n";
		for (int i = 0; i < TotalConnections; i++)
		{
			if (i == ID){
				continue;
			}
			if (sendProtocol(i, protocol)){
				cout << "Przeslano Invite do klienta o ID = " << ID << "\n";
			}
			else{
				cout << "Wystapil problem z wyslaniem Invite do uzytkownika o ID = " << ID << "\n";
			}
		}
		break;
	}
	case PacketHeader::InviteAccpet:
	{
		cout << "Klient o ID = " << ID << " wyslal wiadomosc InviteAccept\n";
		for (int i = 0; i < TotalConnections; i++)
		{
			if (i == ID) {
				continue;
			}
			if (sendProtocol(i, protocol)) {
				cout << "Przeslano InviteAccept do klienta o ID = " << ID << "\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem InviteAccept do uzytkownika o ID = " << ID << "\n";
			}
		}
		break;
	}
	case PacketHeader::InviteDecline:
	{
		cout << "Klient o ID = " << ID << " wyslal wiadomosc InviteIgnore\n";
		for (int i = 0; i < TotalConnections; i++)
		{
			if (i == ID) {
				continue;
			}
			if (sendProtocol(i, protocol)) {
				cout << "Przeslano InviteIgnore do klienta o ID = " << ID << "\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem InviteIgnore do uzytkownika o ID = " << ID << "\n";
			}
		}
		break;
	}
	default:
		cout << "Error, Received unknown packet type!" << endl;
		break;
	}
	return true;
}

bool Server::sendProtocol(int ID, Protocol * protocol)
{
	char * message = protocol->getMessageToSend();
	int messageLenght = protocol->getMessageSize();
	//Send string in strSize bytes
	int check = send(connections[ID], message, messageLenght, NULL);
	if (check == SOCKET_ERROR) //If sending failed
		return false;
	return true;
}

Protocol *  Server::recvProtocol(int ID)
{
	Protocol * protocol;
	int bufferSize = 1500;
	char * buffer = new char[bufferSize];
	//recv packet header and data size
	int check = recv(connections[ID], buffer, bufferSize, NULL);
	if (check == SOCKET_ERROR)
		return nullptr;
	protocol = new Protocol(buffer,9);
	u_int64 dataSize = protocol->getDataSize();
	delete protocol;
	protocol = new Protocol(buffer, 17 + dataSize);
	delete[] buffer;

	return protocol;
}

void Server::ClientHandlerThread(int ID) //Connecions array index
{
	Protocol * protocol;
	while (true)
	{
		protocol = serverPtr->recvProtocol(ID);
		if (protocol == nullptr) //If cannot receive packet properly
			break; //Break infinite loop
		if (!serverPtr->processProtocol(ID, protocol)) //If packet is not properly processed
			break; //Break infinite loop	
	}
	std::cout << "Lost connection to client ID: " << ID << "." << std::endl;
	closesocket(serverPtr->connections[ID]); //Close socked used by client after error occured 
}

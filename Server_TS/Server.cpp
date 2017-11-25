#include "stdafx.h"
#include "Server.h"

vector<Client*> Server::clients;

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
	if (::bind(sListen, (SOCKADDR*)&addr, addrLen) == SOCKET_ERROR) //Bind address to the socket
	{
		cout << "Blad bindowania: " << WSAGetLastError() <<".\n";
	}
	if (listen(sListen, SOMAXCONN) == SOCKET_ERROR) //sListen socket is listening
	{
		cout << "Blad nasluchiwania: " << WSAGetLastError() << ".\n";
	}
	serverPtr = this;
}

bool Server::ListenForNewConnections()
{
	SOCKET newConnection; //Hold's client connection
	newConnection = accept(sListen, (SOCKADDR*)&addr, &addrLen); //Accept a new connection
	if (newConnection == 0) //If accepting the client connection failed
	{
		std::cout << "Blad przy akceptacji klienta: "<< WSAGetLastError() <<".\n";
		return false;
	}
	else //If client connection accepted
	{
		default_random_engine engine(static_cast<long unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count()));
		uniform_int_distribution<int> distro(50, 700);

		Client* newClient = new Client(newConnection,distro(engine));
		std::cout << "Klient polaczony!" << std::endl;
		clients.push_back(newClient);
		//connections[TotalConnections] = newConnection;
		//Create Thread to handle this client. Value i is the index to connections array
		//thread();
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)newClient, NULL, NULL);
		return true;
	}
}

bool Server::processProtocol(Client * client, Protocol * protocol)
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
		sendProtocol(client, new Protocol(PacketHeader::ServerSendID, 0, "", client->sessionID));
		cout << "Wysloano klientowi ID sesji: " << client->sessionID << ".\n";
		break;
	}
	case PacketHeader::Quit:
	{
		cout << "Klient o ID sesji: " << client->sessionID << " rozlaczyl sie.\n";
		//closesocket(client->clientSocket);
		for (auto a : clients)
		{
			if (a == client) {
				continue;
			}
			if (sendProtocol(a, new Protocol(PacketHeader::ServerAlone,0,"",a->sessionID))) {
				cout << "Przeslano wiadomosc do klienta o ID = " << a->sessionID << ".\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem SerwerAlone do uzytkownika o ID = " << a->sessionID << ".\n";
			}
		}
		break;
	}
	case PacketHeader::Message:
	{
		for (auto a : clients)
		{
			if (a == client) {
				continue;
			}
			if (sendProtocol(a, protocol)) {
				cout << "Przeslano wiadomosc do klienta o ID = " << a->sessionID << ".\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem wiadomosci do uzytkownika o ID = " << a->sessionID << ".\n";
			}
		}
		break;
	}
	case PacketHeader::Bye:
	{
		for (auto a : clients)
		{
			if (a == client) {
				continue;
			}
			if (sendProtocol(a, protocol)) {
				cout << "Przeslano Bye do klienta o ID = " << a->sessionID << ".\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem Bye do uzytkownika o ID = " << a->sessionID << ".\n";
			}
		}
		break;
	}
	case PacketHeader::Invite:
	{
		cout << "Klient o ID = " << client->sessionID << " wyslal wiadomosc Invite\n";
		if (clients.size() == 1)
		{
			if (sendProtocol(client, new Protocol(PacketHeader::ServerAlone,0,"",client->sessionID))) {
				cout << "Przeslano ServerAlone do klienta o ID = " << client->sessionID << "\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem ServerAlone do uzytkownika o ID = " << client->sessionID << "\n";
			}
			break;
		}
		for (auto a : clients)
		{
			if (a == client){
				continue;
			}
			if (sendProtocol(a, protocol)){
				cout << "Przeslano Invite do klienta o ID = " << a->sessionID << "\n";
			}
			else{
				cout << "Wystapil problem z wyslaniem Invite do uzytkownika o ID = " << a->sessionID << "\n";
			}
		}
		break;
	}
	case PacketHeader::InviteAccpet:
	{
		cout << "Klient o ID = " << client->sessionID << " wyslal wiadomosc InviteAccept\n";
		for (auto a : clients)
		{
			if (a == client) {
				continue;
			}
			if (sendProtocol(a, protocol)) {
				cout << "Przeslano InviteAccept do klienta o ID = " << a->sessionID << "\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem InviteAccept do uzytkownika o ID = " << a->sessionID << "\n";
			}
		}
		break;
	}
	case PacketHeader::InviteDecline:
	{
		cout << "Klient o ID = " << client->sessionID << " wyslal wiadomosc InviteDecline\n";
		for (auto a : clients)
		{
			if (a == client) {
				continue;
			}
			if (sendProtocol(a, protocol)) {
				cout << "Przeslano InviteDecline do klienta o ID = " << a->sessionID << "\n";
			}
			else {
				cout << "Wystapil problem z wyslaniem InviteDecline do uzytkownika o ID = " << a->sessionID << "\n";
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

bool Server::sendProtocol(Client * client, Protocol * protocol)
{
	char * message = protocol->getMessageToSend();
	int messageLenght = protocol->getMessageSize();
	//Send string in strSize bytes
	int check = send(client->clientSocket, message, messageLenght, NULL);
	if (check == SOCKET_ERROR) //If sending failed
		return false;
	return true;
}

Protocol *  Server::recvProtocol(Client * client)
{
	Protocol * protocol;
	int bufferSize = 1500;
	char * buffer = new char[bufferSize];
	//recv packet header and data size
	int check = recv(client->clientSocket, buffer, bufferSize, NULL);
	if (check == SOCKET_ERROR || check == 0)
		return nullptr;
	protocol = new Protocol(buffer,9);
	u_int64 dataSize = protocol->getDataSize();
	delete protocol;
	protocol = new Protocol(buffer, 17 + dataSize);
	delete[] buffer;

	return protocol;
}

void Server::ClientHandlerThread(Client * client) //Connecions array index
{
	Protocol * protocol;
	while (true)
	{
		protocol = serverPtr->recvProtocol(client);
		if (protocol == nullptr) //If cannot receive packet properly
			break; //Break infinite loop
		if (!serverPtr->processProtocol(client, protocol)) //If packet is not properly processed
			break; //Break infinite loop	
	}
	std::cout << "Rozlaczono z klientem o ID sesji: " << client->sessionID << "." << std::endl;
	if(client->clientSocket != INVALID_SOCKET)
		closesocket(client->clientSocket); //Close socked used by client after error occured
	clients.erase(remove(clients.begin(),clients.end(),client),clients.end());
	delete client;
}

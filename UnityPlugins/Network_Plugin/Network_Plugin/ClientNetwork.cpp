#include "ClientNetwork.h"


//constructor wrapper
NETWORK_H ClientNetwork* CreateClient() {
	return new ClientNetwork();
}
//destructor wrapper
NETWORK_H void DeleteClient(ClientNetwork* client) {
	delete client;
}
//connection
NETWORK_H bool Connect(char* ip, ClientNetwork* client)
{
	return client->connectToServer(ip);
}

NETWORK_H bool SendData(int type, char* message, bool useTCP, ClientNetwork* client)
{
	return client->sendData((PacketType)type, message, useTCP);
}

NETWORK_H void StartUpdating(ClientNetwork* client)
{
	client->startUpdates();
}

NETWORK_H void SetupPacketReception(void(*action)(int type, int sender, char* data))
{
	recievePacket = action;
}

NETWORK_H int GetPlayerNumber(ClientNetwork* client)
{
	return client->index;
}

NETWORK_H int GetError(ClientNetwork* client)
{
	return client->GetError();
}

NETWORK_H int GetErrorLoc(ClientNetwork* client)
{
	return client->GetErrorLoc();
}


ClientNetwork::ClientNetwork()
{
	//1: Start Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);

	//startup
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0) {
		// std::cout << "Failed to Start Winsock" << wsOk;
		error = WSAGetLastError();
		errorLoc = 0;
		return;
	}

	//2. setup server information
	memset(&hintsTCP, 0, sizeof(hintsTCP));
	hintsTCP.ai_family = AF_INET;
	hintsTCP.ai_socktype = SOCK_STREAM;
	hintsTCP.ai_protocol = IPPROTO_TCP;
	//serverTCP.sin_family = AF_INET;
	//serverTCP.sin_port = htons(54223);
	//tcp = socket(serverTCP.sin_family, SOCK_STREAM, IPPROTO_TCP);
	
	memset(&hintsUDP, 0, sizeof(hintsUDP));
	hintsUDP.ai_family = AF_INET;
	hintsUDP.ai_socktype = SOCK_DGRAM;
	hintsUDP.ai_protocol = IPPROTO_UDP;
	//serverlength = sizeof(serverUDP);
	//serverUDP.sin_family = AF_INET;
	//serverUDP.sin_port = htons(54222);
	//udp = socket(serverUDP.sin_family, SOCK_DGRAM, IPPROTO_UDP);
}

ClientNetwork::~ClientNetwork()
{
	listening = false;
	closesocket(tcp);
	closesocket(udp);
	WSACleanup();
}


bool ClientNetwork::connectToServer(std::string ip)
{
	if (ip != "")
	{
		ipActual = ip;
	}

	// Converting string ip address to actually ip address
	//inet_pton(serverTCP.sin_family, ip.c_str(), &serverTCP.sin_addr);
	//connecting to the tcp server
	
	if (getaddrinfo(ipActual.c_str(), "5000", &hintsTCP, &ptrTCP))
	{
		//std::cout << "Getaddrinfo TCP Failed! " << WSAGetLastError() << std::endl;
		error = WSAGetLastError();
		errorLoc = 1;
		WSACleanup();
		return false;
	}

	tcp = socket(ptrTCP->ai_family, ptrTCP->ai_socktype, ptrTCP->ai_protocol);
	if (tcp == INVALID_SOCKET)
	{
		// std::cout << "Can't create TCP socket, Err #" << WSAGetLastError() << std::endl;
		error = WSAGetLastError();
		errorLoc = 2;
		WSACleanup();
		return false;
	}

	
	if (getaddrinfo(ipActual.c_str(), "5001", &hintsUDP, &ptrUDP))
	{
		error = WSAGetLastError();
		errorLoc = 3;
		WSACleanup();
		return false;
	}

	udp = socket(ptrUDP->ai_family, ptrUDP->ai_socktype, ptrUDP->ai_protocol);
	if (udp == INVALID_SOCKET)
	{
		//std::cout << "Can't create UDP socket, Err #" << WSAGetLastError() << std::endl;
		error = WSAGetLastError();
		errorLoc = 4;
		WSACleanup();
		return false;
	}

	if (connect(tcp, ptrTCP->ai_addr, (int)ptrTCP->ai_addrlen) == SOCKET_ERROR)
	{
		//std::cout << "TCP Socket failed to connect to server, Err #" << WSAGetLastError() << std::endl;
		error = WSAGetLastError();
		errorLoc = 5;
		closesocket(tcp);
		freeaddrinfo(ptrTCP);
		WSACleanup();
		return false;
	}

	//ping and determine client index
	return true;
}

bool ClientNetwork::sendData(int packetType, std::string message, bool useTCP)
{
	//create packet
	Packet packet;
	strcpy_s(packet.data, message.c_str() + '\0');
	packet.packet_type = packetType;
	packet.sender = index;

	//set size
	const unsigned int packet_size = sizeof(packet);
	char packet_data[packet_size];

	//seralize
	packet.serialize(packet_data);

	//udp send
	if (!useTCP) {
		if (sendto(udp, packet_data, packet_size, 0, ptrUDP->ai_addr, (int)ptrUDP->ai_addrlen) == SOCKET_ERROR) {
			//std::cout << "Send Error: " << WSAGetLastError() << std::endl;
			error = WSAGetLastError();
			errorLoc = 6;
			return false;
		}
	}
	//tcp send
	else {
		if (send(tcp, packet_data, packet_size, 0) == SOCKET_ERROR)
		{
			//std::cout << "Send Error: " << WSAGetLastError() << std::endl;
			error = WSAGetLastError();
			errorLoc = 7;
			return false;
		}
	}
	return true;
}

void ClientNetwork::startUpdates()
{
	//multithread
	std::thread udpUpdate = std::thread([&]() {
		char* buf = new char[MAX_PACKET_SIZE];

		while (listening) {
			//recieve messages
			int length = recv(udp, buf, MAX_PACKET_SIZE, 0);
			if (length != SOCKET_ERROR) {
				Packet packet;
				packet.deserialize(buf);
				if (packet.packet_type == INIT_CONNECTION) {
					std::vector<std::string> parsedData;
					parsedData = tokenize(',', packet.data);

					index = std::stoi(parsedData[0]);
				}
				else {
					recievePacket(packet.packet_type, packet.sender, packet.data);
				}
			}
		}
	});
	udpUpdate.detach();

	std::thread tcpUpdate = std::thread([&]() {
		char* buf = new char[MAX_PACKET_SIZE];

		while (listening) {
			//recieve packets
			int length = recv(tcp, buf, MAX_PACKET_SIZE, 0);
			if (length != SOCKET_ERROR) {
				Packet packet;
				packet.deserialize(buf);
				if (packet.packet_type == INIT_CONNECTION) {
					std::vector<std::string> parsedData;
					parsedData = tokenize(',', packet.data);

					index = std::stoi(parsedData[0]);

					recievePacket(packet.packet_type, -1, packet.data);

					//connect to udp
					//inet_pton(serverUDP.sin_family, ipActual.c_str(), &serverUDP.sin_addr);

					sendData(INIT_CONNECTION, std::to_string(index), false);
				}
				else {
					recievePacket(packet.packet_type, packet.sender, packet.data);
				}
			}
		}

	});
	tcpUpdate.detach();

}
std::vector<std::string> ClientNetwork::tokenize(char token, std::string text)
{
	std::vector<std::string> temp;
	int lastTokenLocation = 0;

	for (int i = 0; i < text.size(); i++) {
		if (text[i] == token) {
			temp.push_back(text.substr(lastTokenLocation, i - lastTokenLocation));
			lastTokenLocation = i + 1;

		}
	}
	temp.push_back(text.substr(lastTokenLocation, text.size()));

	return temp;
}

int ClientNetwork::GetError()
{
	return error;
}

int ClientNetwork::GetErrorLoc()
{
	return errorLoc;
}


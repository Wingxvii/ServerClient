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

NETWORK_H void SetupPacketReception(void(*action)(char* buffer, int length, bool TCP))
{
	receivePacket = action;
}

NETWORK_H void StartUpdating(ClientNetwork* client)
{
	client->startUpdates();
}

NETWORK_H void SendDebugOutput(char* data)
{
	printf(data);
	printf("\n");
}

NETWORK_H bool SendDataPacket(char* ptr, int length, bool TCP, ClientNetwork* client)
{
	return client->sendDataPacket(ptr, length, TCP);
}

NETWORK_H int GetError(ClientNetwork* client)
{
	return client->GetError();
}

NETWORK_H int GetErrorLoc(ClientNetwork* client)
{
	return client->GetErrorLoc();
}

NETWORK_H void ShowConsole(ClientNetwork* client, bool open)
{
	client->ShowConsole(open);
}

NETWORK_H void UpdateFile(ClientNetwork* client)
{
	client->UpdateFile();
}

NETWORK_H void ClearFile(ClientNetwork* client)
{
	client->ClearFile();
}

NETWORK_H void Reset(ClientNetwork* client)
{
	client->Reset();
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

	ClearFile();
}

ClientNetwork::~ClientNetwork()
{
	listening = false;
	closesocket(tcp);
	closesocket(udp);
	WSACleanup();
	ShowConsole(false);
}

bool ClientNetwork::connectToServer(std::string ip)
{	
	if (socketInit) {
		return true;
	}

	if (ip != "")
	{
		serverIP = ip;
	}

	std::cout << "ConnectingTCP..." << std::endl;
	if (getaddrinfo(serverIP.c_str(), "55555", &hintsTCP, &ptrTCP) != 0)
	{
		//std::cout << "Getaddrinfo TCP Failed! " << WSAGetLastError() << std::endl;
		error = WSAGetLastError();
		errorLoc = 1;
		UpdateFile();
		WSACleanup();
		return false;
	}

	tcp = socket(ptrTCP->ai_family, ptrTCP->ai_socktype, ptrTCP->ai_protocol);
	if (tcp == INVALID_SOCKET)
	{
		// std::cout << "Can't create TCP socket, Err #" << WSAGetLastError() << std::endl;
		error = WSAGetLastError();
		errorLoc = 2;
		UpdateFile();
		WSACleanup();
		return false;
	}

	if (connect(tcp, ptrTCP->ai_addr, (int)ptrTCP->ai_addrlen) == SOCKET_ERROR)
	{
		//std::cout << "TCP Socket failed to connect to server, Err #" << WSAGetLastError() << std::endl;
		error = WSAGetLastError();
		errorLoc = 5;
		UpdateFile();
		closesocket(tcp);
		freeaddrinfo(ptrTCP);
		WSACleanup();

		std::cout << "Connection Failed!" << std::endl;

		return false;
	}
	else
	{
		socketInit = true;
	}
	std::cout << "TCP Connected!" << std::endl;

	std::cout << "ConnectingUDP..." << std::endl;
	if (getaddrinfo(serverIP.c_str(), "60000", &hintsUDP, &ptrUDP) != 0)
	{
		error = WSAGetLastError();
		errorLoc = 3;
		UpdateFile();
		WSACleanup();
		return false;
	}

	udp = socket(ptrUDP->ai_family, ptrUDP->ai_socktype, ptrUDP->ai_protocol);
	if (udp == INVALID_SOCKET)
	{
		//std::cout << "Can't create UDP socket, Err #" << WSAGetLastError() << std::endl;
		error = WSAGetLastError();
		errorLoc = 4;
		UpdateFile();
		WSACleanup();
		return false;
	}
	std::cout << "Connected UDP!" << std::endl;

	//ping and determine client index
	init = true;
	return true;
}

bool ClientNetwork::sendDataPacket(char* ptr, int length, bool TCP)
{
	if (init)
	{
		if (!TCP) {
			//PrintPackInfo(1, ptr, length);
			if (sendto(udp, ptr, DEFAULT_DATA_SIZE, 0, ptrUDP->ai_addr, (int)ptrUDP->ai_addrlen) == SOCKET_ERROR) {
				//std::cout << "Send Error: " << WSAGetLastError() << std::endl;
				error = WSAGetLastError();
				errorLoc = 6;
				UpdateFile();
				return false;
			}
		}
		//tcp send
		else {
			if (send(tcp, ptr, DEFAULT_DATA_SIZE, 0) == SOCKET_ERROR)
			{
				//std::cout << "Send Error: " << WSAGetLastError() << std::endl;
				error = WSAGetLastError();
				errorLoc = 7;
				UpdateFile();
				return false;
			}
		}
		return true;
	}
	return false;
}

void ClientNetwork::startUpdates()
{
	std::thread tcpUpdate = std::thread([&]()
		{
			char* buff = new char[DEFAULT_DATA_SIZE];

			while (listening) {
				//receive packets
				int sError = recv(tcp, buff, DEFAULT_DATA_SIZE, 0);
				if (sError != SOCKET_ERROR) {
					int pLength = 0;
					int ok_pack = 0;
					memcpy(&ok_pack, &buff[0] + PACKET_STAMP, sizeof(ok_pack));
					if (ok_pack == OK_STAMP)
					{
						memcpy(&pLength, &buff[0] + PACKET_LENGTH, sizeof(pLength));

						//PrintPackInfo(-1, buff, pLength);
						receivePacket(buff, pLength, true);
					}
				}
			}

			delete[] buff;
		});
	tcpUpdate.detach();


	std::thread udpUpdate = std::thread([&]()
		{
			char* buff = new char[DEFAULT_DATA_SIZE];

			while (listening) {
				//receive messages
				int sError = recv(udp, buff, DEFAULT_DATA_SIZE, 0);

				if (sError != SOCKET_ERROR) {
					int ok_pack = 0;
					memcpy(&ok_pack, &buff[0] + PACKET_STAMP, sizeof(ok_pack));
					if (ok_pack == OK_STAMP)
					{
						int pLength = 0;
						memcpy(&pLength, &buff[0] + PACKET_LENGTH, sizeof(pLength));
						//PrintPackInfo(-1, buff, pLength);
						receivePacket(buff, pLength, false);
					}
				}
			}

			delete[] buff;
		});
	udpUpdate.detach();
}

int ClientNetwork::GetError()
{
	return error;
}

int ClientNetwork::GetErrorLoc()
{
	return errorLoc;
}

std::string ClientNetwork::GetErrorText()
{
	return errorText;
}

void ClientNetwork::ShowConsole(bool open)
{
	if (open && !consoleOpen)
	{
		FILE* pConsole;
		AllocConsole();
		freopen_s(&pConsole, "CONOUT$", "wb", stdout);
		consoleOpen = true;
	}
	else if (!open && consoleOpen)
	{
		FreeConsole();
		consoleOpen = false;
	}
}

void ClientNetwork::UpdateFile()
{
	std::ofstream saveFile;
	saveFile.open(filePath);
	if (ClientNetwork::GetErrorLoc() >= 100)
	{
		saveFile << "Error Loc: " << ClientNetwork::GetErrorLoc()
			<< "\t Error: " << ClientNetwork::GetErrorText() << std::endl;
	}
	else
	{
		saveFile << "Error Loc: " << ClientNetwork::GetErrorLoc()
			<< "\t Error: " << ClientNetwork::GetError() << std::endl;
	}

	saveFile.close();
}

void ClientNetwork::ClearFile()
{
	std::ofstream saveFile;
	saveFile.open(filePath);
	saveFile.clear();
	saveFile.close();
}

void ClientNetwork::Reset()
{

}

// Printing Packet Data
//void ClientNetwork::PrintPackInfo(int sender, char* data, int dataLen)
//{
//	std::cout << "Sender: " << sender << ", " << dataLen << ", Data: ";
//	for (int i = 0; i < dataLen; ++i)
//	{
//		std::cout << (int)data[i] << "\t";
//	}
//	std::cout << "\nend" << std::endl;
//}
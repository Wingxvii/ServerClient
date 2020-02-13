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

NETWORK_H bool SendDataInit(packet_init pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(PacketType::INIT, (char*)&pkt, sizeof(packet_init), useTCP);
}

NETWORK_H bool SendDataJoin(packet_join pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(PacketType::JOIN, (char*)&pkt, sizeof(packet_join), useTCP);
}

NETWORK_H bool SendDataMsg(packet_msg pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(PacketType::MESSAGE, (char*)&pkt, sizeof(packet_msg), useTCP);
}

NETWORK_H bool SendDataState(packet_state pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(PacketType::STATE, (char*)&pkt, sizeof(packet_state), useTCP);
}

NETWORK_H bool SendDataEntity(packet_entity pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(PacketType::ENTITY, (char*)&pkt, sizeof(packet_entity), useTCP);
}

NETWORK_H bool SendDataDamage(packet_damage pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(PacketType::DAMAGE, (char*)&pkt, sizeof(packet_damage), useTCP);
}

NETWORK_H bool SendDataWeapon(packet_weapon pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(PacketType::WEAPON, (char*)&pkt, sizeof(packet_weapon), useTCP);
}

NETWORK_H bool SendDataBuild(packet_build pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(PacketType::BUILD, (char*)&pkt, sizeof(packet_build), useTCP);
}

NETWORK_H bool SendDataKill(packet_kill pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(PacketType::DEATH, (char*)&pkt, sizeof(packet_kill), useTCP);
}

NETWORK_H void SendDebugOutput(char* data)
{
	printf(data);
	printf("\n");
}

NETWORK_H bool SendDataPacket(char* ptr, int length, bool TCP, ClientNetwork* client)
{
	std::cout << "DLL Sending Packet" << std::endl;
	return client->sendDataPacket(ptr, length, TCP);
}



//NETWORK_H void SetupPacketReception(void(*action)(int sender, char* data))
//{
//	receivePacket = action;
//}

NETWORK_H void SetupPacketReceptionInit(void(*action)(int sender, packet_init data))
{
	receivePacketInit = action;
}

NETWORK_H void SetupPacketReceptionJoin(void(*action)(int sender, packet_join data))
{
	receivePacketJoin = action;
}

NETWORK_H void SetupPacketReceptionMsg(void(*action)(int sender, packet_msg data))
{
	receivePacketMsg = action;
}

NETWORK_H void SetupPacketReceptionState(void(*action)(int sender, packet_state data))
{
	receivePacketState = action;
}

NETWORK_H void SetupPacketReceptionEntity(void(*action)(int sender, packet_entity data))
{
	receivePacketEntity = action;
}

NETWORK_H void SetupPacketReceptionDamage(void(*action)(int sender, packet_damage data))
{
	receivePacketDamage = action;
}

NETWORK_H void SetupPacketReceptionWeapon(void(*action)(int sender, packet_weapon data))
{
	receivePacketWeapon = action;
}

NETWORK_H void SetupPacketReceptionBuild(void(*action)(int sender, packet_build data))
{
	receivePacketBuild = action;
}

NETWORK_H void SetupPacketReceptionKill(void(*action)(int sender, packet_kill data))
{
	receivePacketKill = action;
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

	FILE* pConsole;
	AllocConsole();
	freopen_s(&pConsole, "CONOUT$", "wb", stdout);
}

ClientNetwork::~ClientNetwork()
{
	listening = false;
	closesocket(tcp);
	closesocket(udp);
	WSACleanup();
	FreeConsole();
}


bool ClientNetwork::connectToServer(std::string ip)
{	
	if (ip != "")
	{
		serverIP = ip;
	}
	//std::cout << ip << std::endl;

	// Converting string ip address to actually ip address
	//inet_pton(serverTCP.sin_family, ip.c_str(), &serverTCP.sin_addr);
	//connecting to the tcp server

	std::cout << "ConnectingTCP..." << std::endl;
	if (getaddrinfo(serverIP.c_str(), "5000", &hintsTCP, &ptrTCP) != 0)
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

	std::cout << "Connected!" << std::endl;
	std::cout << "ConnectingUDP..." << std::endl;

	if (getaddrinfo(serverIP.c_str(), "5001", &hintsUDP, &ptrUDP) != 0)
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
	std::cout << "Init value: " << (init ? "true" : "false") << std::endl;
	return true;
}

bool ClientNetwork::sendData(int packetType, char* data, size_t size, bool useTCP)
{
	if (init)
	{
		//create packet
		Packet packet;
		memcpy(packet.data, &data, size);
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
				UpdateFile();
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
				UpdateFile();
				return false;
			}
		}
		return true;
	}
	return false;
}

bool ClientNetwork::sendDataPacket(char* ptr, int length, bool TCP)
{
	std::cout << "Sending Packets..." << (int)init << std::endl;
	if (init)
	{
		std::cout << "Sending Inited" << std::endl;
		if (!TCP) {
			std::cout << "SENDING UDP DATA" << std::endl;
			PrintPackInfo(1, ptr, length);
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
			std::cout << "SENDING TCP DATA" << std::endl;
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
	std::cout << "Start Receiving..." << std::endl;
	std::thread tcpUpdate = std::thread([&]()
		{
			char* buff = new char[DEFAULT_DATA_SIZE];

			std::cout << "Started TCP" << std::endl;
			while (listening) {
				//receive packets
				int sError = recv(tcp, buff, DEFAULT_DATA_SIZE, 0);
				if (sError != SOCKET_ERROR) {
					int pLength = 0;

					memcpy(&pLength, &buff[0], sizeof(pLength));

					PrintPackInfo(-1, buff, pLength);
					receivePacket(buff, pLength, true);
				}
			}

			delete[] buff;
		});
	tcpUpdate.detach();


	std::thread udpUpdate = std::thread([&]()
		{
			std::cout << "Started UDP" << std::endl;
			char* buff = new char[DEFAULT_DATA_SIZE];

			while (listening) {
				//receive messages
				int sError = recv(udp, buff, DEFAULT_DATA_SIZE, 0);
				if (sError != SOCKET_ERROR) {
					int pLength = 0;
					memcpy(&pLength, &buff[0], sizeof(pLength));
					PrintPackInfo(-1, buff, pLength);
					receivePacket(buff, pLength, false);
				}
			}

			delete[] buff;
		});
	udpUpdate.detach();
}
/*
void ClientNetwork::startUpdates()
{
	if (init)
	{
		//multithread
		std::thread udpUpdate = std::thread([&]()
			{
				char* buf = new char[MAX_PACKET_SIZE];

				while (listening) {
					//receive messages
					int length = recv(udp, buf, MAX_PACKET_SIZE, 0);
					if (length != SOCKET_ERROR) {
						Packet packet;
						packet.deserialize(buf);
						if (packet.packet_type == INIT) {
							packet_init init;
							memcpy(&init, &packet.data, sizeof(packet_init));
							//std::cout << "UDP Packet INIT ERROR@@@: " << init.index << std::endl;
						}
						else if (packet.packet_type == JOIN) {
							packet_join join;
							memcpy(&join, &packet.data, sizeof(packet_join));
							// index = join.playerID;
							//std::cout << "UDP Packet JOIN ERROR@@@: " << join.playerID << std::endl;
						}
						else {
							switch (packet.packet_type)
							{
							case PacketType::MESSAGE:
								packet_msg msg;
								memcpy(&msg, &packet.data, sizeof(packet_msg));
								receivePacketMsg(packet.sender, msg);
								break;
							case PacketType::STATE:
								packet_state state;
								memcpy(&state, &packet.data, sizeof(packet_state));
								receivePacketState(packet.sender, state);
								break;
							case PacketType::ENTITY:
								packet_entity entity;
								memcpy(&entity, &packet.data, sizeof(packet_entity));
								receivePacketEntity(packet.sender, entity);
								break;
							case PacketType::DAMAGE:
								packet_damage dmg;
								memcpy(&dmg, &packet.data, sizeof(packet_damage));
								receivePacketDamage(packet.sender, dmg);
								break;
							case PacketType::WEAPON:
								packet_weapon weapon;
								memcpy(&weapon, &packet.data, sizeof(packet_weapon));
								receivePacketWeapon(packet.sender, weapon);
								break;
							case PacketType::BUILD:
								packet_build build;
								memcpy(&build, &packet.data, sizeof(packet_build));
								receivePacketBuild(packet.sender, build);
								break;
							case PacketType::DEATH:
								packet_kill kill;
								memcpy(&kill, &packet.data, sizeof(packet_kill));
								receivePacketKill(packet.sender, kill);
								break;
							default:
								errorLoc = 100;
								errorText = "Invalid Packet Type: " + packet.packet_type;
								break;
							}
							// receivePacket(packet.packet_type, packet.sender, packet.data);
						}
					}
				}
			});
		udpUpdate.detach();

		std::thread tcpUpdate = std::thread([&]()
			{
				char* buf = new char[MAX_PACKET_SIZE];

				while (listening) {
					//receive packets
					int length = recv(tcp, buf, MAX_PACKET_SIZE, 0);
					if (length != SOCKET_ERROR) {
						Packet packet;
						packet.deserialize(buf);
						if (packet.packet_type == INIT) {
							if (packet.sender == -1)
							{
								packet_init init;
								memcpy(&init, &packet.data, sizeof(packet_init));
								index = init.index;

								sendData(INIT, (char*)&init, sizeof(packet_init), false);
							}
							else
							{
								errorLoc = 11;
								errorText = "PACKET INIT Not from Server : " + std::to_string(packet.sender);
								UpdateFile();
							}
						}
						else if (packet.packet_type == JOIN)
						{
							packet_join join;
							memcpy(&join, &packet.data, sizeof(packet_join));
							index = join.playerID;
						}
						else {
							switch (packet.packet_type)
							{
							case PacketType::MESSAGE:
								packet_msg msg;
								memcpy(&msg, &packet.data, sizeof(packet_msg));
								receivePacketMsg(packet.sender, msg);
								break;
							case PacketType::STATE:
								packet_state state;
								memcpy(&state, &packet.data, sizeof(packet_state));
								receivePacketState(packet.sender, state);
								break;
							case PacketType::ENTITY:
								packet_entity entity;
								memcpy(&entity, &packet.data, sizeof(packet_entity));
								receivePacketEntity(packet.sender, entity);
								break;
							case PacketType::DAMAGE:
								packet_damage dmg;
								memcpy(&dmg, &packet.data, sizeof(packet_damage));
								receivePacketDamage(packet.sender, dmg);
								break;
							case PacketType::WEAPON:
								packet_weapon weapon;
								memcpy(&weapon, &packet.data, sizeof(packet_weapon));
								receivePacketWeapon(packet.sender, weapon);
								break;
							case PacketType::BUILD:
								packet_build build;
								memcpy(&build, &packet.data, sizeof(packet_build));
								receivePacketBuild(packet.sender, build);
								break;
							case PacketType::DEATH:
								packet_kill kill;
								memcpy(&kill, &packet.data, sizeof(packet_kill));
								receivePacketKill(packet.sender, kill);
								break;
							default:
								errorLoc = 100;
								errorText = "Invalid Packet Type: " + packet.packet_type;
								break;
							}

							// receivePacket(packet.packet_type, packet.sender, packet.data); // old
						}
					}
				}

			});
		tcpUpdate.detach();
	}
}
*/
std::vector<std::string> ClientNetwork::tokenize(char token, std::string text)
{
	std::vector<std::string> temp;
	int lastTokenLocation = 0;

	for (int i = 0; i < text.size(); i++) {
		if (text[i] == token) {
			temp.push_back(text.substr(lastTokenLocation, (size_t)(i - lastTokenLocation)));
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

std::string ClientNetwork::GetErrorText()
{
	return errorText;
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


void ClientNetwork::PrintPackInfo(int sender, char* data, int dataLen)
{
	std::cout << "Sender: " << sender << ", " << dataLen << ", Data: ";
	for (int i = 0; i < dataLen; ++i)
	{
		std::cout << (int)data[i] << "\t";
	}
	std::cout << "\nend" << std::endl;
}
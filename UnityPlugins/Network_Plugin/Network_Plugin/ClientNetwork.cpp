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

NETWORK_H bool SendDataInit(int type, packet_init pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(type, (char*)&pkt, sizeof(packet_init), useTCP);
}

NETWORK_H bool SendDataJoin(int type, packet_join pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(type, (char*)&pkt, sizeof(packet_join), useTCP);
}

NETWORK_H bool SendDataMsg(int type, packet_msg pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(type, (char*)&pkt, sizeof(packet_msg), useTCP);
}

NETWORK_H bool SendDataState(int type, packet_state pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(type, (char*)&pkt, sizeof(packet_state), useTCP);
}

NETWORK_H bool SendDataEntity(int type, packet_entity pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(type, (char*)&pkt, sizeof(packet_entity), useTCP);
}

NETWORK_H bool SendDataDamage(int type, packet_damage pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(type, (char*)&pkt, sizeof(packet_damage), useTCP);
}

NETWORK_H bool SendDataWeapon(int type, packet_weapon pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(type, (char*)&pkt, sizeof(packet_weapon), useTCP);
}

NETWORK_H bool SendDataBuild(int type, packet_build pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(type, (char*)&pkt, sizeof(packet_build), useTCP);
}

NETWORK_H bool SendDataKill(int type, packet_kill pkt, bool useTCP, ClientNetwork* client)
{
	return client->sendData(type, (char*)&pkt, sizeof(packet_kill), useTCP);
}

NETWORK_H void StartUpdating(ClientNetwork* client)
{
	client->startUpdates();
}

//NETWORK_H void SetupPacketReception(void(*action)(int type, int sender, char* data))
//{
//	receivePacket = action;
//}

NETWORK_H void SetupPacketReceptionInit(void(*action)(int type, int sender, packet_init data))
{
	receivePacketInit = action;
}

NETWORK_H void SetupPacketReceptionJoin(void(*action)(int type, int sender, packet_join data))
{
	receivePacketJoin = action;
}

NETWORK_H void SetupPacketReceptionMsg(void(*action)(int type, int sender, packet_msg data))
{
	receivePacketMsg = action;
}

NETWORK_H void SetupPacketReceptionState(void(*action)(int type, int sender, packet_state data))
{
	receivePacketState = action;
}

NETWORK_H void SetupPacketReceptionEntity(void(*action)(int type, int sender, packet_entity data))
{
	receivePacketEntity = action;
}

NETWORK_H void SetupPacketReceptionDamage(void(*action)(int type, int sender, packet_damage data))
{
	receivePacketDamage = action;
}

NETWORK_H void SetupPacketReceptionWeapon(void(*action)(int type, int sender, packet_weapon data))
{
	receivePacketWeapon = action;
}

NETWORK_H void SetupPacketReceptionBuild(void(*action)(int type, int sender, packet_build data))
{
	receivePacketBuild = action;
}

NETWORK_H void SetupPacketReceptionKill(void(*action)(int type, int sender, packet_kill data))
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

NETWORK_H packet_init GetInit(ClientNetwork* client)
{
	return client->GetInit();
}

NETWORK_H bool SetInit(ClientNetwork* client, packet_init packet)
{
	return client->SetInit(packet);
}

NETWORK_H packet_msg GetMsg(ClientNetwork* client)
{
	return client->GetMsg();
}

NETWORK_H bool SetMsg(ClientNetwork* client, packet_msg packet)
{
	return client->SetMsg(packet);
}

NETWORK_H packet_entity GetEntity(ClientNetwork* client)
{
	return client->GetEntity();
}

NETWORK_H bool SetEntity(ClientNetwork* client, packet_entity packet)
{
	return client->SetEntity(packet);
}

NETWORK_H packet_weapon GetWeapon(ClientNetwork* client)
{
	return client->GetWeapon();
}

NETWORK_H bool SetWeapon(ClientNetwork* client, packet_weapon packet)
{
	return client->SetWeapon(packet);
}

NETWORK_H packet_damage GetDamage(ClientNetwork* client)
{
	return client->GetDamage();
}

NETWORK_H bool SetDamage(ClientNetwork* client, packet_damage packet)
{
	return client->SetDamage(packet);
}

NETWORK_H packet_build GetBuild(ClientNetwork* client)
{
	return client->GetBuild();
}

NETWORK_H bool SetBuild(ClientNetwork* client, packet_build packet)
{
	return client->SetBuild(packet);
}

NETWORK_H packet_kill GetKill(ClientNetwork* client)
{
	return client->GetKill();
}

NETWORK_H bool SetKill(ClientNetwork* client, packet_kill packet)
{
	return client->SetKill(packet);
}

NETWORK_H packet_state GetState(ClientNetwork* client)
{
	return client->GetState();
}

NETWORK_H bool SetState(ClientNetwork* client, packet_state packet)
{
	return client->SetState(packet);
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


	if (getaddrinfo(ipActual.c_str(), "5001", &hintsUDP, &ptrUDP))
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

	if (connect(tcp, ptrTCP->ai_addr, (int)ptrTCP->ai_addrlen) == SOCKET_ERROR)
	{
		//std::cout << "TCP Socket failed to connect to server, Err #" << WSAGetLastError() << std::endl;
		error = WSAGetLastError();
		errorLoc = 5;
		UpdateFile();
		closesocket(tcp);
		freeaddrinfo(ptrTCP);
		WSACleanup();
		return false;
	}

	//ping and determine client index
	return true;
}

bool ClientNetwork::sendData(int packetType, char* data, size_t size, bool useTCP)
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

void ClientNetwork::startUpdates()
{
	//multithread
	std::thread udpUpdate = std::thread([&]() {
		char* buf = new char[MAX_PACKET_SIZE];

		while (listening) {
			//receive messages
			int length = recv(udp, buf, MAX_PACKET_SIZE, 0);
			if (length != SOCKET_ERROR) {
				Packet packet;
				packet.deserialize(buf);
				if (packet.packet_type == JOIN) {
					//std::vector<std::string> parsedData;
					//parsedData = tokenize(',', packet.data);

					//index = std::stoi(parsedData[0]);
					packet_join join;
					memcpy(&join, &packet.data, sizeof(join));
					index = join.playerID;
				}
				else {
					switch (packet.packet_type)
					{
					case PacketType::MESSAGE:
						packet_msg msg;
						memcpy(&msg, &packet.data, sizeof(packet_msg));
						receivePacketMsg(packet.packet_type, packet.sender, msg);
						break;
					case PacketType::STATE:
						packet_state state;
						memcpy(&state, &packet.data, sizeof(packet_state));
						receivePacketState(packet.packet_type, packet.sender, state);
						break;
					case PacketType::ENTITY:
						packet_entity entity;
						memcpy(&entity, &packet.data, sizeof(packet_entity));
						receivePacketEntity(packet.packet_type, packet.sender, entity);
						break;
					case PacketType::DAMAGE:
						packet_damage dmg;
						memcpy(&dmg, &packet.data, sizeof(packet_damage));
						receivePacketDamage(packet.packet_type, packet.sender, dmg);
						break;
					case PacketType::WEAPON:
						packet_weapon weapon;
						memcpy(&weapon, &packet.data, sizeof(packet_weapon));
						receivePacketWeapon(packet.packet_type, packet.sender, weapon);
						break;
					case PacketType::BUILD:
						packet_build build;
						memcpy(&build, &packet.data, sizeof(packet_build));
						receivePacketBuild(packet.packet_type, packet.sender, build);
						break;
					case PacketType::KILL:
						packet_kill kill;
						memcpy(&kill, &packet.data, sizeof(packet_kill));
						receivePacketKill(packet.packet_type, packet.sender, kill);
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

	std::thread tcpUpdate = std::thread([&]() {
		char* buf = new char[MAX_PACKET_SIZE];

		while (listening) {
			//receive packets
			int length = recv(tcp, buf, MAX_PACKET_SIZE, 0);
			if (length != SOCKET_ERROR) {
				Packet packet;
				packet.deserialize(buf);
				if (packet.packet_type == JOIN) {
					//std::vector<std::string> parsedData;
					//parsedData = tokenize(',', packet.data);
					//
					//index = std::stoi(parsedData[0]);
					//receivePacket(packet.packet_type, -1, packet.data);

					packet_join join;
					memcpy(&join, &packet.data, sizeof(packet_join));
					index = join.playerID;

					//connect to udp
					//inet_pton(serverUDP.sin_family, ipActual.c_str(), &serverUDP.sin_addr);

					sendData(INIT, (char*)&join, sizeof(packet_join), false);
				}
				else {
					switch (packet.packet_type)
					{
					case PacketType::MESSAGE:
						packet_msg msg;
						memcpy(&msg, &packet.data, sizeof(packet_msg));
						receivePacketMsg(packet.packet_type, packet.sender, msg);
						break;
					case PacketType::STATE:
						packet_state state;
						memcpy(&state, &packet.data, sizeof(packet_state));
						receivePacketState(packet.packet_type, packet.sender, state);
						break;
					case PacketType::ENTITY:
						packet_entity entity;
						memcpy(&entity, &packet.data, sizeof(packet_entity));
						receivePacketEntity(packet.packet_type, packet.sender, entity);
						break;
					case PacketType::DAMAGE:
						packet_damage dmg;
						memcpy(&dmg, &packet.data, sizeof(packet_damage));
						receivePacketDamage(packet.packet_type, packet.sender, dmg);
						break;
					case PacketType::WEAPON:
						packet_weapon weapon;
						memcpy(&weapon, &packet.data, sizeof(packet_weapon));
						receivePacketWeapon(packet.packet_type, packet.sender, weapon);
						break;
					case PacketType::BUILD:
						packet_build build;
						memcpy(&build, &packet.data, sizeof(packet_build));
						receivePacketBuild(packet.packet_type, packet.sender, build);
						break;
					case PacketType::KILL:
						packet_kill kill;
						memcpy(&kill, &packet.data, sizeof(packet_kill));
						receivePacketKill(packet.packet_type, packet.sender, kill);
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

packet_init ClientNetwork::GetInit()
{

	return packet_init();
}

bool ClientNetwork::SetInit(packet_init packet)
{

	return false;
}

packet_msg ClientNetwork::GetMsg()
{
	return packet_msg();
}

bool ClientNetwork::SetMsg(packet_msg packet)
{
	return false;
}

packet_entity ClientNetwork::GetEntity()
{
	return packet_entity();
}

bool ClientNetwork::SetEntity(packet_entity packet)
{
	return false;
}

packet_weapon ClientNetwork::GetWeapon()
{
	return packet_weapon();
}

bool ClientNetwork::SetWeapon(packet_weapon packet)
{
	return false;
}

packet_damage ClientNetwork::GetDamage()
{
	return packet_damage();
}

bool ClientNetwork::SetDamage(packet_damage packet)
{
	return false;
}

packet_build ClientNetwork::GetBuild()
{
	return packet_build();
}

bool ClientNetwork::SetBuild(packet_build packet)
{
	return false;
}

packet_kill ClientNetwork::GetKill()
{
	return packet_kill();
}

bool ClientNetwork::SetKill(packet_kill packet)
{
	return false;
}

packet_state ClientNetwork::GetState()
{
	return packet_state();
}

bool ClientNetwork::SetState(packet_state packet)
{
	return false;
}

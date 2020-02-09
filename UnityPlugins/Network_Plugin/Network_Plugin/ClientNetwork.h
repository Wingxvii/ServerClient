#pragma once

#ifdef PLUGIN_EXPORTS
#define NETWORK_H __declspec(dllexport)
#elif PLUGIN_IMPORTS
#define NETWORK_H __declspec(dllimport)
#else
#define NETWORK_H
#endif

#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include <fstream>
#include "Packet.h"

#pragma comment (lib, "ws2_32.lib")

struct Vec3 {
	float x;
	float y;
	float z;
};


#pragma once
#pragma once
class ClientNetwork
{
public:

	ClientNetwork();
	~ClientNetwork();

	SOCKET udp;
	SOCKET tcp;

	struct addrinfo* ptrUDP = NULL, hintsUDP;
	struct addrinfo* ptrTCP = NULL, hintsTCP;
	//sockaddr_in serverUDP;
	//sockaddr_in serverTCP;
	//int serverlength = 0;

	bool listening = true;
	bool init = false;

	//client details
	std::string filePath = "ErrorLog.txt";
	std::string ipActual = "127.0.0.1";
	int index = 0;
	int error = 0;
	int errorLoc = 0;
	std::string errorText = "";
public:

	bool connectToServer(std::string ip);

	bool sendData(int packetType, char* data, size_t size, bool useTCP);

	// int sendMessage(std::string message, bool useTCP = false);

	void startUpdates();
	//bool sendData(int packetType, std::string message, bool useTCP = false);	//udp send data

	//tokenizes into string vects
	static std::vector<std::string> tokenize(char token, std::string text);

	int GetError();
	int GetErrorLoc();
	std::string GetErrorText();

	void UpdateFile();
	void ClearFile();
	void Reset();
};

extern "C" {
	//message action
	// void (*receivePacket)(int type, int sender, char* data);

	void (*receivePacketInit)(int sender, packet_init data);
	void (*receivePacketJoin)(int sender, packet_join data);
	void (*receivePacketMsg)(int sender, packet_msg data);
	void (*receivePacketState)(int sender, packet_state data);
	void (*receivePacketEntity)(int sender, packet_entity data);
	void (*receivePacketDamage)(int sender, packet_damage data);
	void (*receivePacketWeapon)(int sender, packet_weapon data);
	void (*receivePacketBuild)(int sender, packet_build data);
	void (*receivePacketKill)(int sender, packet_kill data);

	NETWORK_H ClientNetwork* CreateClient();
	NETWORK_H void DeleteClient(ClientNetwork* client);
	NETWORK_H bool Connect(char* ip, ClientNetwork* client);
	NETWORK_H void StartUpdating(ClientNetwork* client);
	// NETWORK_H void SetupPacketReception(void(*action)(int type, int sender, char* data));

	NETWORK_H void SetupPacketReceptionInit(void(*action)(int sender, packet_init data));
	NETWORK_H void SetupPacketReceptionJoin(void(*action)(int sender, packet_join data));
	NETWORK_H void SetupPacketReceptionMsg(void(*action)(int sender, packet_msg data));
	NETWORK_H void SetupPacketReceptionState(void(*action)(int sender, packet_state data));
	NETWORK_H void SetupPacketReceptionEntity(void(*action)(int sender, packet_entity data));
	NETWORK_H void SetupPacketReceptionDamage(void(*action)(int sender, packet_damage data));
	NETWORK_H void SetupPacketReceptionWeapon(void(*action)(int sender, packet_weapon data));
	NETWORK_H void SetupPacketReceptionBuild(void(*action)(int sender, packet_build data));
	NETWORK_H void SetupPacketReceptionKill(void(*action)(int sender, packet_kill data));

	NETWORK_H bool SendDataInit(packet_init pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataJoin(packet_join pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataMsg(packet_msg pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataState(packet_state pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataEntity(packet_entity pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataDamage(packet_damage pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataWeapon(packet_weapon pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataBuild(packet_build pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataKill(packet_kill pkt, bool useTCP, ClientNetwork* client);

	NETWORK_H int GetPlayerNumber(ClientNetwork* client);

	NETWORK_H int GetError(ClientNetwork* client);
	NETWORK_H int GetErrorLoc(ClientNetwork* client);
	NETWORK_H void UpdateFile(ClientNetwork* client);
	NETWORK_H void ClearFile(ClientNetwork* client);
	NETWORK_H void Reset(ClientNetwork* client);
}

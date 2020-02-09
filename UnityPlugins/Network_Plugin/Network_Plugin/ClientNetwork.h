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

	packet_init GetInit();
	bool SetInit(packet_init packet);
	packet_msg GetMsg();
	bool SetMsg(packet_msg packet);
	packet_entity GetEntity();
	bool SetEntity(packet_entity packet);
	packet_weapon GetWeapon();
	bool SetWeapon(packet_weapon packet);
	packet_damage GetDamage();
	bool SetDamage(packet_damage packet);
	packet_build GetBuild();
	bool SetBuild(packet_build packet);
	packet_kill GetKill();
	bool SetKill(packet_kill packet);
	packet_state GetState();
	bool SetState(packet_state packet);
};

extern "C" {
	//message action
	// void (*receivePacket)(int type, int sender, char* data);

	void (*receivePacketInit)(int type, int sender, packet_init data);
	void (*receivePacketJoin)(int type, int sender, packet_join data);
	void (*receivePacketMsg)(int type, int sender, packet_msg data);
	void (*receivePacketState)(int type, int sender, packet_state data);
	void (*receivePacketEntity)(int type, int sender, packet_entity data);
	void (*receivePacketDamage)(int type, int sender, packet_damage data);
	void (*receivePacketWeapon)(int type, int sender, packet_weapon data);
	void (*receivePacketBuild)(int type, int sender, packet_build data);
	void (*receivePacketKill)(int type, int sender, packet_kill data);

	NETWORK_H ClientNetwork* CreateClient();
	NETWORK_H void DeleteClient(ClientNetwork* client);
	NETWORK_H bool Connect(char* ip, ClientNetwork* client);
	NETWORK_H void StartUpdating(ClientNetwork* client);
	// NETWORK_H void SetupPacketReception(void(*action)(int type, int sender, char* data));

	NETWORK_H void SetupPacketReceptionInit(void(*action)(int type, int sender, packet_init data));
	NETWORK_H void SetupPacketReceptionJoin(void(*action)(int type, int sender, packet_join data));
	NETWORK_H void SetupPacketReceptionMsg(void(*action)(int type, int sender, packet_msg data));
	NETWORK_H void SetupPacketReceptionState(void(*action)(int type, int sender, packet_state data));
	NETWORK_H void SetupPacketReceptionEntity(void(*action)(int type, int sender, packet_entity data));
	NETWORK_H void SetupPacketReceptionDamage(void(*action)(int type, int sender, packet_damage data));
	NETWORK_H void SetupPacketReceptionWeapon(void(*action)(int type, int sender, packet_weapon data));
	NETWORK_H void SetupPacketReceptionBuild(void(*action)(int type, int sender, packet_build data));
	NETWORK_H void SetupPacketReceptionKill(void(*action)(int type, int sender, packet_kill data));

	NETWORK_H bool SendDataInit(int type, packet_init pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataJoin(int type, packet_join pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataMsg(int type, packet_msg pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataState(int type, packet_state pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataEntity(int type, packet_entity pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataDamage(int type, packet_damage pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataWeapon(int type, packet_weapon pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataBuild(int type, packet_build pkt, bool useTCP, ClientNetwork* client);
	NETWORK_H bool SendDataKill(int type, packet_kill pkt, bool useTCP, ClientNetwork* client);

	NETWORK_H int GetPlayerNumber(ClientNetwork* client);

	NETWORK_H int GetError(ClientNetwork* client);
	NETWORK_H int GetErrorLoc(ClientNetwork* client);
	NETWORK_H void UpdateFile(ClientNetwork* client);
	NETWORK_H void ClearFile(ClientNetwork* client);
	NETWORK_H void Reset(ClientNetwork* client);

	NETWORK_H packet_init GetInit(ClientNetwork* client);
	NETWORK_H bool SetInit(ClientNetwork* client, packet_init packet);
	NETWORK_H packet_msg GetMsg(ClientNetwork* client);
	NETWORK_H bool SetMsg(ClientNetwork* client, packet_msg packet);
	NETWORK_H packet_entity GetEntity(ClientNetwork* client);
	NETWORK_H bool SetEntity(ClientNetwork* client, packet_entity packet);
	NETWORK_H packet_weapon GetWeapon(ClientNetwork* client);
	NETWORK_H bool SetWeapon(ClientNetwork* client, packet_weapon packet);
	NETWORK_H packet_damage GetDamage(ClientNetwork* client);
	NETWORK_H bool SetDamage(ClientNetwork* client, packet_damage packet);
	NETWORK_H packet_build GetBuild(ClientNetwork* client);
	NETWORK_H bool SetBuild(ClientNetwork* client, packet_build packet);
	NETWORK_H packet_kill GetKill(ClientNetwork* client);
	NETWORK_H bool SetKill(ClientNetwork* client, packet_kill packet);
	NETWORK_H packet_state GetState(ClientNetwork* client);
	NETWORK_H bool SetState(ClientNetwork* client, packet_state packet);
}

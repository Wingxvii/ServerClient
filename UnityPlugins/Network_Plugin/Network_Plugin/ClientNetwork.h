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
	std::string ipActual = "127.0.0.1";
	int index = 0;
	int error = 0;
	int errorLoc = 0;
public:
	bool connectToServer(std::string ip);

	// int sendMessage(std::string message, bool useTCP = false);

	void startUpdates();
	bool sendData(int packetType, std::string message, bool useTCP = false);	//udp send data

	//tokenizes into string vects
	static std::vector<std::string> tokenize(char token, std::string text);

	int GetError();
	int GetErrorLoc();

};

extern "C" {
	//message action
	void (*recievePacket)(int type, int sender, char* data);

	NETWORK_H ClientNetwork* CreateClient();
	NETWORK_H void DeleteClient(ClientNetwork* client);
	NETWORK_H bool Connect(char* ip, ClientNetwork* client);
	NETWORK_H void StartUpdating(ClientNetwork* client);
	NETWORK_H void SetupPacketReception(void(*action)(int type, int sender, char* data));

	NETWORK_H bool SendData(int type, char* message, bool useTCP, ClientNetwork* client);

	NETWORK_H int GetPlayerNumber(ClientNetwork* client);

	NETWORK_H int GetError(ClientNetwork* client);
	NETWORK_H int GetErrorLoc(ClientNetwork* client);
}

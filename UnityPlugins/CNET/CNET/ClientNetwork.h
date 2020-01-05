#pragma once

#define CNET_H _declspec(dllexport)

#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include "Packet.h"

#pragma comment (lib, "ws2_32.lib")

using namespace std;


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

	sockaddr_in serverUDP;
	sockaddr_in serverTCP;
	int serverlength;

	bool listening = true;

	//client details
	string addressDefault = "127.0.0.1";
	string ipActual = "";
	int index = 0;

public:
	int connectToServer();
	int connectToServer(string ip);

	int sendMessage(string message, bool useTCP = false);

	void startUpdates();
	int sendData(int packetType, string message, bool useTCP = false);	//udp send data

	//tokenizes into string vects
	static std::vector<std::string> tokenize(char token, std::string text);

};

extern "C" {
	//message action
	void (*recievePacket)(int type, int sender, char* data);

	CNET_H ClientNetwork* CreateClient();
	CNET_H void DeleteClient(ClientNetwork* client);
	CNET_H void Connect(char* ip, ClientNetwork* client);
	CNET_H void StartUpdating(ClientNetwork* client);
	CNET_H void SetupPacketReception(void(*action)(int type, int sender, char* data));

	CNET_H void SendData(int type, char* message, bool useTCP, ClientNetwork* client);


	CNET_H int GetPlayerNumber(ClientNetwork* client);


}

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
class ClientNetwork
{
public:
	ClientNetwork();
	~ClientNetwork();

	SOCKET client;
	sockaddr_in server;
	int serverlength;

	bool listening = true;

	vector<std::vector<std::string>> connectionsIn;
	vector<std::vector<std::string>> messagesIn;

	//client details
	string addressDefault = "127.0.0.1";
	int index = 0;

public:
	int connect();
	int connect(string ip);

	void startUpdates();
	int sendData(int packetType, string message);

	//tokenizes into string vects
	static std::vector<std::string> tokenize(char token, std::string text);

	//data send wrappers
	int SendTransformation(Vec3 pos, Vec3 rot);


};

extern "C" {
	//message action
	void (*recievePacket)(int type, int sender, char* data);

	void (*DebugCommand)(char* data);

	//shared methods here
	CNET_H int Add(int a, int b);
	CNET_H void RecieveString(const char* str);		//this is send
	CNET_H void SendString(char* str, int length);	//this is recieve


	CNET_H ClientNetwork* CreateClient();
	CNET_H void DeleteClient(ClientNetwork* client);
	CNET_H void Connect(char* ip, ClientNetwork* client);
	CNET_H void StartUpdating(ClientNetwork* client);

	CNET_H void SendData(int type, char* message,  ClientNetwork* client);
	CNET_H void SetupPacketReception(void(*action)(int type, int sender, char* data));
	CNET_H int GetPlayerNumber(ClientNetwork* client);

	CNET_H void SetDebugCommand(void(*action)(char* data));
}

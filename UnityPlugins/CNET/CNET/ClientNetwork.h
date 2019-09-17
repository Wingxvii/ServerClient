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

#pragma once
class ClientNetwork
{
public:
	ClientNetwork();
	~ClientNetwork();

	SOCKET client;
	sockaddr_in server;
	int serverlength;

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
	int sendMessage(string message);


};

struct temp {
	string x = "";
};

extern "C" {
	//shared methods here
	CNET_H int Add(int a, int b);
	CNET_H void RecieveString(const char* str);
	CNET_H void SendString(char* str, int length);


	CNET_H ClientNetwork* CreateClient();
	CNET_H void DeleteClient(ClientNetwork* client);
	CNET_H int Connect(char* ip, ClientNetwork* client);


	CNET_H void SendMsg(char* ip, ClientNetwork* client);

}

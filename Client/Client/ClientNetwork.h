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
	vector<std::vector<std::string>> transformationsIn;

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


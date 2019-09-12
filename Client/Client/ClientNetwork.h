#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include "Packet.h"
#include "Tokenizer.h"

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
	int sendMessage(int packetType, string message);

};


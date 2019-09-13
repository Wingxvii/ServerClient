#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include "Packet.h"
#include "Tokenizer.h"

#pragma comment (lib, "ws2_32.lib")

using namespace std;

struct UserProfile {
	int index;
	string Username;
	sockaddr_in clientAddress;
	string clientIP;
	int clientLength;
};

#pragma once
class ServerNetwork
{
public:
	ServerNetwork();
	~ServerNetwork();

	SOCKET in;
	sockaddr_in serverHint;
	int clientLength;

	vector<std::vector<std::string>> messagesIn;

	int clientCount = 0;

	std::vector<UserProfile> ConnectedUsers;

public:
	//accept and save new socket
	void acceptNewClient(std::vector<std::string> data, sockaddr_in address, int length);
	//begin listening to input signals
	void startUpdates();

	//send to all clients
	void sendToAll(int packetType, string message);
	//send to sepific client
	void sendTo(int packetType, string message, int clientID);
	//send to all except a client
	void sendToAllExcept(int packetType, string message, int clientID);
};


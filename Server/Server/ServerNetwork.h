#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include "Packet.h"

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

	vector<Packet> inQueue;

	int clientCount = 0;

	std::vector<UserProfile> ConnectedUsers;

public:
	//accept and save new socket
	void acceptNewClient(sockaddr_in address, int length);
	//begin listening to input signals
	void startListening();

	//send to all clients
	void sendToAll(string message);
	//send to sepific client
	void sendTo(string message, int clientID);

};


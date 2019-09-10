#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>

#pragma comment (lib, "ws2_32.lib")
#define MAX_PACKET_SIZE 10000

using namespace std;

struct UserProfile {
	int index;
	string Username;
	sockaddr_in clientAddress;
	string clientIP;
};

#pragma once
class ServerNetwork
{
public:
	ServerNetwork();
	~ServerNetwork();

	SOCKET in;
	sockaddr_in serverHint;

	queue<char*> inQueue;

	char buf[MAX_PACKET_SIZE];

	int clientLength;

	std::vector<UserProfile> ConnectedUsers;

public:
	//accept and save new socket
	bool acceptNewClient();
	//begin listening to input signals
	void startListening();

	//send to all clients
	void sendToAll(string message);
	//send to sepific client
	void sendTo(string message, int clientID);


public:

};


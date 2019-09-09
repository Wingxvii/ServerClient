#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>

#pragma comment (lib, "ws2_32.lib")
#define MAX_PACKET_SIZE 10000

using namespace std;

#pragma once
class ServerNetwork
{
public:
	ServerNetwork();
	~ServerNetwork();

	SOCKET in;
	sockaddr_in serverHint;

	queue<char*> inQueue = queue<char*>();

	char buf[MAX_PACKET_SIZE];

	int clientLength;

public:
	bool acceptNewClient(unsigned int& id);
	void startListening();
	int receiveData(unsigned int client_id, char* recvbuf);
	void sendToAll(string message, int totalSize);
	void sendTo(string message, int totalSize, int clientID);

};


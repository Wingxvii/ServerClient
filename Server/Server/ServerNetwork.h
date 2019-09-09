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


public:
	bool acceptNewClient(unsigned int& id);
	int receiveData(unsigned int client_id, char* recvbuf);
	void sendToAll(char* packets, int totalSize);
	void sendTo(char* packets, int totalSize, int clientID);

};


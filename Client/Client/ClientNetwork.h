#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>

#pragma comment (lib, "ws2_32.lib")
#define MAX_PACKET_SIZE 10000

using namespace std;

#pragma once
class ClientNetwork
{
public:
	ClientNetwork();
	~ClientNetwork();

	sockaddr_in server;
	int serverlength;

	SOCKET client;

	string addressDefault = "127.0.0.1";
	char buf[MAX_PACKET_SIZE];

	queue<char*> inQueue = queue<char*>();

	string clientIndex = "01"; //should be -1

public:
	int connect();
	int connect(string ip);

	int sendMessage(string message);

	void startListening();

	void setClientIndex(string index);

	//multithread recieve from as a listener
		//recvfrom waits for a recieved data
		//when data is recieved, push to processing queue, then call self
		//while queue has data, return to process
};


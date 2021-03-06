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

	SOCKET udp;
	SOCKET tcp;

	sockaddr_in serverUDP;
	sockaddr_in serverTCP;
	int serverlength;

	bool listening = true;

	//client details
	string addressDefault = "127.0.0.1";
	string ipActual = "";
	int index = 0;

public:
	int connectToServer();
	int connectToServer(string ip);

	void startUpdates();
	int sendData(int packetType, string message, bool useTCP = false);	//udp send data

	//tokenizes into string vects
	static std::vector<std::string> tokenize(char token, std::string text);

	void ProcessTCP(Packet pack);
	void ProcessUDP(Packet pack);


};
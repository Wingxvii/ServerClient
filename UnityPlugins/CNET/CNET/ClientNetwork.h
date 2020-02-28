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

	string username = "test";
	bool inGame = false;
	bool requestActive = false;
	int requesterIndex = -1;

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

extern "C" {
	//message action

	CNET_H ClientNetwork* CreateClient();
	CNET_H void DeleteClient(ClientNetwork* client);
	CNET_H void Connect(char* ip, char* username, ClientNetwork* client);
	CNET_H void StartUpdating(ClientNetwork* client);

	CNET_H void SendData(int type, char* message, bool useTCP, ClientNetwork* client);

	//functions for assignment

	//c++ client var gets
	CNET_H int GetPlayerNumber(ClientNetwork* client);
	CNET_H bool GetInGame(ClientNetwork* client);
	CNET_H bool GetRequestActive(ClientNetwork* client);
	CNET_H int GetRequesterIndex(ClientNetwork* client);

	//request session from other clients

	CNET_H void RequestGame(int index, ClientNetwork* client);
	CNET_H void RespondToRequest(bool acceptance, ClientNetwork* client);

	//quit active session
	CNET_H void QuitGame(ClientNetwork* client);

	//data requests from the server
	CNET_H void RequestLobbyData(ClientNetwork* client);
	CNET_H void RequestSessionData(ClientNetwork* client);


	//actions

	//recieve packet
	void (*UDPMessage)(int sender, char* data);
	CNET_H void SetupUDPMessage(void(*action)(int sender, char* data));

	//on connect
	void (*onConnect)();
	CNET_H void SetupOnConnect(void(*action)());

	//on message
	void (*onMessage)(char* data);
	CNET_H void SetupOnMessage(void(*action)());


}

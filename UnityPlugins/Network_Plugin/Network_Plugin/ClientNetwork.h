#pragma once

#ifdef PLUGIN_EXPORTS
#define NETWORK_H __declspec(dllexport)
#elif PLUGIN_IMPORTS
#define NETWORK_H __declspec(dllimport)
#else
#define NETWORK_H
#endif

#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <queue>
#include <fstream>
#include "Packet.h"

class ClientNetwork
{
public:
	ClientNetwork();
	~ClientNetwork();

	SOCKET udp;
	SOCKET tcp;

	struct addrinfo* ptrUDP = NULL, hintsUDP;
	struct addrinfo* ptrTCP = NULL, hintsTCP;
	//sockaddr_in serverUDP;
	//sockaddr_in serverTCP;

	bool listening = true;
	bool init = false;
	bool socketInit = false;
	bool consoleOpen = false;

	//client details
	std::string filePath = "ErrorLog.txt";
	std::string serverIP = "127.0.0.1";
	int index = 0;
	int error = 0;
	int errorLoc = 0;
	std::string errorText = "";
public:

	bool connectToServer(std::string ip);

	//bool sendData(int packetType, char* data, size_t size, bool useTCP);

	bool sendDataPacket(char* ptr, int length, bool TCP);

	void startUpdates();

	int GetError();
	int GetErrorLoc();
	std::string GetErrorText();

	//void PrintPackInfo(int sender, char* data, int datalen);
	void ShowConsole(bool open);

	void UpdateFile();
	void ClearFile();
	void Reset();
};

extern "C" {
	//message action
	// void (*receivePacket)(int type, int sender, char* data);

	void (*receivePacket)(char* buffer, int length, bool TCP);

	NETWORK_H ClientNetwork* CreateClient();
	NETWORK_H void DeleteClient(ClientNetwork* client);
	NETWORK_H bool Connect(char* ip, ClientNetwork* client);
	NETWORK_H void SetupPacketReception(void(*action)(char* buffer, int length, bool TCP));
	NETWORK_H void StartUpdating(ClientNetwork* client);
	NETWORK_H bool SendDataPacket(char* ptr, int length, bool TCP, ClientNetwork* client);

	//NETWORK_H int GetPlayerNumber(ClientNetwork* client);

	NETWORK_H void SendDebugOutput(char* data);
	NETWORK_H int GetError(ClientNetwork* client);
	NETWORK_H int GetErrorLoc(ClientNetwork* client);
	NETWORK_H void ShowConsole(ClientNetwork* client, bool open);
	NETWORK_H void UpdateFile(ClientNetwork* client);
	NETWORK_H void ClearFile(ClientNetwork* client);
	NETWORK_H void Reset(ClientNetwork* client);
}

#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include "Packet.h"
#include "Tokenizer.h"
#include "GameData.h"

#pragma comment (lib, "ws2_32.lib")
#define DEFAULT_PORT "6883" 

struct UserProfile {
	int index;
	std::string Username;
	sockaddr_in udpAddress;
	SOCKET tcpSocket;
	std::string clientIP;
	int clientLength;

	//checks for disconnection
	bool active = false;
};

#pragma once
class ServerNetwork
{
public:
	ServerNetwork();
	~ServerNetwork();

	//UDP Socket
	SOCKET udp;
	//struct addrinfo* ptrUDP = NULL, hintsUDP;
	sockaddr_in serverUDP;
	//int clientLength;

	SOCKET tcp;
	//struct addrinfo* ptrTCP = NULL, hintsTCP;
	sockaddr_in serverTCP;
	//master list of tracked TCP sockets
	fd_set master;

	bool listening = true;
	bool rtsExists = false;

	std::vector<Packet> packetsIn;

	int clientCount = 0;

	std::vector<UserProfile> ConnectedUsers;
	
	//std::vector<EntityData> entities;

public:
	//initalize the entity game data
	void initEntities();

	//accept and save new socket
	void acceptNewClient(int sender, sockaddr_in address, int length);

	//begin listening to input signals
	void startUpdates();

	//send to all clients
	void sendToAll(Packet pack);
	//send to sepific client(udp) (should not be used)
	void sendTo(Packet pack, int clientID);


	//send to all except a client
	void relay(Packet pack, bool useTCP = false);
	//print to cout
	void printOut(Packet pack, int clientID);
	//tcp send to
	void sendTo(Packet pack, SOCKET client);

	void SwapIndex(int current, int target);

	void ProcessTCP(Packet pack);
	void ProcessUDP(Packet pack);
};


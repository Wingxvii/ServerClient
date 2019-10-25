#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include "Packet.h"
#include "Tokenizer.h"

#pragma comment (lib, "ws2_32.lib")
#define DEFAULT_PORT "6883" 

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

	bool listening = true;

	vector<Packet> packetsIn;

	vector<Packet> packets1In;	//RTS
	vector<Packet> packets2In;	//FPS1
	vector<Packet> packets3In;	//FPS2
	vector<Packet> packets4In;	//FPS3


	int clientCount = 0;

	std::vector<UserProfile> ConnectedUsers;

public:
	//accept and save new socket
	void acceptNewClient(std::vector<std::string> data, sockaddr_in address, int length);
	//begin listening to input signals
	void startUpdates();

	//send to all clients
	void sendToAll(Packet pack);
	//send to sepific client
	void sendTo(Packet pack, int clientID);
	//send to all except a client
	void relay(Packet pack, int clientID);
	//print to cout
	void printOut(Packet pack, int clientID);


};


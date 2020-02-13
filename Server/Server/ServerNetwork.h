#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include "Packet.h"
#include "Tokenizer.h"
#include "GameData.h"
#include <chrono>

#pragma comment (lib, "ws2_32.lib")
#define DEFAULT_PORT "6883" 
#define INITIAL_OFFSET 16

struct UserProfile {
	int index;
	std::string Username;
	sockaddr_in udpAddress;
	SOCKET tcpSocket;
	std::string clientIP;
	int clientLength;

	PlayerType type;
	bool ready;
	bool loaded;

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
	bool allReady = false;
	bool gameLoading = false;
	bool allLoaded = false;
	int rtsPlayers = 0;
	int fpsPlayers = 0;
	int clientCount = 0;

	float timeOut = 0.f;

	float maxTimer = 5.0f;
	float timer = maxTimer;

	float deltaTime;
	std::chrono::system_clock::time_point previousTime;

	std::vector<Packet> packetsIn;

	std::vector<UserProfile> ConnectedUsers;
	
	//std::vector<EntityData> entities;

public:
	//initalize the entity game data
	void initEntities();

	//accept and save new socket
	void acceptNewClient(int sender, sockaddr_in address, int length);

	void SetReady(bool readyState);

	//begin listening to input signals
	void startUpdates();

	void PrintPackInfo(const char* extraInfo, int sender, char* data, int datalen);

	template<class T>
	void PackData(char* buffer, int* loc, T data);

	void PackString(char* buffer, int* loc, std::string* str);

	template<class T>
	void UnpackData(char* buffer, int* loc, T* data);

	void UnpackString(char* buffer, int* loc, std::string* str, int *length);

	void PackAuxilaryData(char* buffer, int length, int receiver, int type, int sender = -1);

	bool ChangeType(PlayerType requestedType);

	void packetTCP(char* packet);

	void packetUDP(char* packet, sockaddr_in fromAddr, int fromLen);


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

template<class T>
inline void ServerNetwork::PackData(char* buffer, int* loc, T data)
{
	memcpy(buffer + *loc, &data, sizeof(T));
	*loc += sizeof(T);
}

template<class T>
inline void ServerNetwork::UnpackData(char* buffer, int* loc, T* data)
{
	memcpy(data, buffer + *loc, sizeof(T));
	*loc += sizeof(T);
}

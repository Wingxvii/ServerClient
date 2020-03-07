#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include "Packet.h"
#include "Tokenizer.h"
#include "GameData.h"
#include <chrono>
#include <fstream>

#pragma comment (lib, "ws2_32.lib")
#define DEFAULT_PORT "55555" 
#define PACKET_STAMP 0
#define PACKET_LENGTH 4
#define PACKET_RECEIVERS 8
#define PACKET_TYPE 12
#define PACKET_SENDER 16
#define INITIAL_OFFSET 20
#define OK_STAMP 123456789

struct UserProfile {
	int index;
	std::string Username;
	SOCKET tcpSocket;
	sockaddr_in tcpAddress;
	int tcpLength;
	sockaddr_in udpAddress;
	std::string clientIP;
	int udpLength;

	PlayerType type;
	bool ready;
	bool loaded;

	//checks for disconnection
	bool active = false;
	bool activeUDP = false;
};

class GameData
{
public:
	GameData();
	~GameData();

	void reset();

	int* built;
	int* died;

	std::string MetricsFile = "USER_METRICS.txt";

	bool alreadyCompleted = false;
};

#pragma once
class ServerNetwork
{
public:
	ServerNetwork();
	~ServerNetwork();

	GameData gd;

	//UDP Socket
	SOCKET udp;
	sockaddr_in serverUDP;
	//struct addrinfo* ptrUDP = NULL, hintsUDP;
	//int clientLength;

	SOCKET tcp;
	sockaddr_in serverTCP;
	//struct addrinfo* ptrTCP = NULL, hintsTCP;
	//master list of tracked TCP sockets
	fd_set master;

	bool listening = true;

	// State checker
	bool allReady = false;
	bool gameLoading = false;
	bool allLoaded = false;
	bool gameEnded = false;

	int rtsPlayers = 0;
	int fpsPlayers = 0;
	int clientCount = 0;
	int clientLimit = 4;

	float timeOut = 0.f;

	float maxTimer = 5.0f;
	float timer = maxTimer;

	float deltaTime;
	std::chrono::system_clock::time_point previousTime;

	//std::vector<Packet> packetsIn;

	std::vector<UserProfile> ConnectedUsers;
	
	//std::vector<EntityData> entities;

public:
	//initalize the entity game data
	//void initEntities();
	void acceptTCPConnection();

	//accept and save new socket
	void acceptNewClient(int sender, sockaddr_in address, int length);

	void SetReady(bool readyState);

	void StartGame();

	void EndGame();

	void StartLoading(float timer);

	void SocketListening(SOCKET sock);

	//begin listening to input signals
	void startUpdates();

	void PrintPackInfo(const char* extraInfo, int sender, char* data, int datalen);

	bool ChangeType(PlayerType requestedType);

	void packetTCP(char* packet);

	void packetUDP(char* packet, sockaddr_in fromAddr, int fromLen);

	void SwapIndex(int current, int target);

	void PackAuxilaryData(char* buffer, int length, int receiver, int type, int sender = -1);

	void PackString(char* buffer, int* loc, std::string* str);

	template<class T>
	void PackData(char* buffer, int* loc, T data);

	template<class T>
	void UnpackData(char* buffer, int* loc, T* data);

	void UnpackString(char* buffer, int* loc, std::string* str, int *length);

	void UpdateMetrics(bool ignoreCompletion);
	////send to all clients
	//void sendToAll(Packet pack);
	////send to sepific client(udp) (should not be used)
	//void sendTo(Packet pack, int clientID);
	////send to all except a client
	//void relay(Packet pack, bool useTCP = false);
	////print to cout
	//void printOut(Packet pack, int clientID);
	////tcp send to
	//void sendTo(Packet pack, SOCKET client);
	//void ProcessTCP(Packet pack);
	//void ProcessUDP(Packet pack);
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

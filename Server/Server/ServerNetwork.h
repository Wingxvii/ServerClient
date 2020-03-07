#pragma once
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
#define MAX_SAVE_FILES 99

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

class ServerNetwork
{
public:
	ServerNetwork();
	~ServerNetwork();


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

struct Death
{
	int killer_type;
	Vector3 death_loc;
	float alive_timer;
};

struct Damage
{
	int attacker_type;
	Vector3 location;
	float damage;
};

struct Buildings
{
	Vector3 build_locations;
	int type;
};

struct Transaction
{
	float amount;
	int target;
};

struct PlayerData
{
	float alive_timer;

	// Basic Info
	std::string player_name;
	PlayerType player_type;
	
	float total_damage_dealt;
	int total_kills;
	float total_credits_earned;
	float total_credits_spent;
	
	std::vector<int> kill_target_types;
	std::vector<Transaction> credit_earned;
	std::vector<Transaction> credit_spent;

	// FPS
	std::vector<Death> deaths;
	std::vector<Damage> damages;
	std::vector<Vector3> location_tracking;

	// RTS
	int total_turrets;
	int total_barracks;
	int total_droids;
	
	std::vector<Buildings> buildings;
};

class UserMetrics
{
public:
	// Singleton
	static UserMetrics* getInstance() {
		if (!instance) {
			instance = new UserMetrics();
		}
		return instance;
	}
	void initialize(std::vector<UserProfile> connected_users);

	void recordDamage(int id, Damage dmg);
	void recordDeath(int id, Death death);
	void recordKill(int id, int target_type);
	void recordBuild(int id, Buildings building);
	void recordEarning(int id, Transaction trans);
	void recordSpending(int id, Transaction trans);

	void writeToFile();

private:
	std::string file_name = "UserMetrics";
	std::string file_type = ".txt";
	std::string current_file_path;
	static UserMetrics* instance;
	UserMetrics()
		:init(false), winner(-1), session_id(0)
	{
	};

	bool init;

	// Winners RTS = 0, FPS = 1
	int winner;
	int session_id;

	std::vector<std::shared_ptr<PlayerData>> player_list;
};
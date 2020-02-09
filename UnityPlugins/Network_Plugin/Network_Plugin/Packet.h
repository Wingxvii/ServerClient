#pragma once
#define MAX_PACKET_SIZE 6000
#define DEFAULT_DATA_SIZE 5700

#include <stdio.h>
#include <stdlib.h>   
#include <string>
#include <ws2tcpip.h>

#pragma comment (lib, "ws2_32.lib")
//
//enum PacketType {
//	//initialization connection
//	INIT = 0,
//	//single string
//	MESSAGE = 1,
//
//	//FPS Managed Data
//	//data of players
//	PLAYER_DATA = 2,
//	//player weapon switch
//	WEAPON_STATE = 3,
//	//environment damage
//	DAMAGE_DEALT = 4,
//
//	//RTS Managed Data
//
//	//data of all droids (up to 100)
//	ENTITY_DATA = 5, // Entity Data
//	//entity built
//	BUILD = 6,
//	//entity killed
//	KILL = 7,
//	//game state
//	GAME_STATE = 8,
//	//player damaged
//	PLAYER_DAMAGE = 9,
//	//data of all turrets
//	TURRET_DATA = 10
//};

enum PacketType {
	// initialization connection
	INIT = 0,
	// Join the Game
	JOIN,
	// single string
	MESSAGE,
	// game state
	STATE,

	// Entity Data
	ENTITY,
	// Damage dealt (int ID, bool Dir, int source, float damage)
	DAMAGE,

	// FPS weapon switch
	WEAPON,

	//RTS Managed Data
	// entity built
	BUILD,
	// entity killed
	KILL
};

struct Packet {

	unsigned int packet_type;
	int sender = 0;
	char data[DEFAULT_DATA_SIZE];
	unsigned int id = 0;

	void serialize(char* data) {
		memcpy(data, this, sizeof(Packet));
	}

	void deserialize(char* data) {
		memcpy(this, data, sizeof(Packet));
	}
};

NETWORK_H struct packet_init {
	bool init;
};

NETWORK_H struct packet_join {
	int playerID;
};

NETWORK_H struct packet_msg {
	char* message;
};

NETWORK_H struct entity {
	float posX;
	float posY;
	float posZ;
	float rotX;
	float rotY;
	float rotZ;
	int state;
};

NETWORK_H struct packet_entity {
	entity entities[500];
};

NETWORK_H struct packet_weapon {
	int weapon;
};

NETWORK_H struct packet_damage {
	int playerID;
	bool dir;
	int entity;
	float damage;
};

NETWORK_H struct packet_build {
	int id;
	int type;
	float posX;
	float posY;
	float posZ;
};

NETWORK_H struct packet_kill {
	int id;
};

NETWORK_H struct packet_state {
	int state;
};
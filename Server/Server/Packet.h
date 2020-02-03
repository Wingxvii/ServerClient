#pragma once
#define MAX_PACKET_SIZE 6000
#define DEFAULT_DATA_SIZE 5700

#include <stdio.h>
#include <stdlib.h>   
#include <string>
#include <ws2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

enum PacketType {
	//initialization connection
	INIT_CONNECTION = 0,
	//single string
	MESSAGE = 1,

	//FPS Managed Data

	//data of players
	PLAYER_DATA = 2,
	//player weapon switch
	WEAPON_DATA = 3,
	//environment damage
	ENVIRONMENT_DAMAGE = 4,
	
	//RTS Managed Data

	//data of all droids (up to 100)
	DROID_POSITION = 5,
	//entity built
	BUILD_ENTITY = 6,
	//entity killed
	KILL_ENTITY = 7,
	//game state
	GAME_STATE = 8,
	//player damaged
	PLAYER_DAMAGE = 9,
	//data of all turrets
	TURRET_DATA = 10

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


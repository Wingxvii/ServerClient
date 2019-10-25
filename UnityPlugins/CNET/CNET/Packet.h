#pragma once
#define MAX_PACKET_SIZE 100000
#define DEFAULT_DATA_SIZE 3500

#include <stdio.h>
#include <stdlib.h>   
#include <string>


enum PacketType {
	//initialization connection
	INIT_CONNECTION = 0,

	//single string
	MESSAGE = 1,

	//FPS DATA TYPES

	//2vec3 + int
	PLAYERDATA = 2,
	//int
	WEAPONSTATE = 3,
	//int
	DAMAGEDEALT = 4,

	//RTS DATA TYPES

	//vec4[0-100]
	DROIDLOCATIONS = 5,
	//2int + vec3
	BUILD = 6,
	//int
	KILL = 7,
	//int
	GAMESTATE = 8,

};

struct Packet {

	unsigned int packet_type;
	unsigned int sender = 0;
	char data[DEFAULT_DATA_SIZE];
	unsigned int id = 0;

	void serialize(char* data) {
		memcpy(data, this, sizeof(Packet));
	}

	void deserialize(char* data) {
		memcpy(this, data, sizeof(Packet));
	}
};


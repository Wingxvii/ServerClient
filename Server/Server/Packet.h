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
	INIT_CONNECTION,
	//single string
	MESSAGE,
	//player's position udp
	PLAYER_POSITION,
	//puck position for non-physics player udp
	PUCK_POSITIONS,
	//puck impulse for physics player tcp
	PUCK_IMPULSE,
	//info about game win and loss tcp
	GAMEWIN,

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


#pragma once
#define DEFAULT_PACKET_SIZE 1024

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


};

struct Packet {

	unsigned int packet_type;
	unsigned int sender = 0;
	char data[DEFAULT_PACKET_SIZE];

	void serialize(char* data) {
		memcpy(data, this, sizeof(Packet));
	}

	void deserialize(char* data) {
		memcpy(this, data, sizeof(Packet));
	}
};


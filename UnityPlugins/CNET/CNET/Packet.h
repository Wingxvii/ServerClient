#pragma once
#define MAX_PACKET_SIZE 100000
#define DEFAULT_DATA_SIZE 1482

#include <stdio.h>
#include <stdlib.h>   
#include <string>


enum PacketType {
	//initialization connection
	INIT_CONNECTION = 0,

	//single string
	MESSAGE = 1,

	//transformation
	TRANSFORMATION = 2,



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


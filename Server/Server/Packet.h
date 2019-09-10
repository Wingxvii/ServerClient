#pragma once
#define MAX_PACKET_SIZE 1000000
#define DEFAULT_PACKET_SIZE 512
#define LARGER_PACKET_SIZE 8192

#include <stdio.h>
#include <stdlib.h>   
#include <string.h>

/*
Format of a Packet:

char packet = [0][1][2][3+]
[0][1] = User Index (00 - 99)
[2][3] = Packet Type (@PacketType)
[4] = size
[5+] = Data

*/

enum PacketSize {
	//default
	SIZE_DEFAULT = 0,

	//larger
	SIZE_BIG = 1,

	//max size
	SIZE_MAXIMUM = 2,

};

enum PacketType {
	//initialization connection
	INIT_CONNECTION = 0,

	//single string
	MESSAGE = 1,


};

class Packet
{
public:
	Packet();
	Packet(char* dataIn);
	~Packet();

public:

	int size();
	char* getPacketData();
	int getPacketType();
	int getUserIndex();

private:
	bool checked = false;
	char* buf;
	int index;
	PacketType type;
	PacketSize size;

};


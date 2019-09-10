#pragma once
#define DEFAULT_PACKET_SIZE 1024

#include <stdio.h>
#include <stdlib.h>   
#include <string.h>
#include <ws2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

/*
Format of a Packet:

char packet = [0][1][2][3+]
[0][1] = User Index (00 - 99)
[2][3] = Packet Type (@PacketType)
[4+] = Data

*/

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

	char* getPacketData();
	PacketType getPacketType();
	int getUserIndex();

	sockaddr_in source;
	int sourceLength;
private:
	char* buf;
	int index;
	PacketType type;

};


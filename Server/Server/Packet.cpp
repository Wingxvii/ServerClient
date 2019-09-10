#include "Packet.h"

Packet::Packet()
{
	char temp[DEFAULT_PACKET_SIZE] = { '0','0','0','0','0'};
	*this = Packet(temp);
}

Packet::Packet(char* dataIn)
{
	char indexChar[2] = { dataIn[0], dataIn[1] };
	char typeChar[2] = { dataIn[2], dataIn[3] };

	index = atoi(indexChar);
	type = (PacketType)atoi(typeChar);
	buf = new char[DEFAULT_PACKET_SIZE];

	strcpy_s(buf, DEFAULT_PACKET_SIZE - 4, dataIn + 4);
}


Packet::~Packet()
{
}

char* Packet::getPacketData()
{
	return buf;
}

PacketType Packet::getPacketType()
{
	return type;
}

int Packet::getUserIndex()
{
	return index;
}

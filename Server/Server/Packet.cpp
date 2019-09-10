#include "Packet.h"

Packet::Packet()
{
	char temp[DEFAULT_PACKET_SIZE] = { '0','0','0','0','0','0'};
	*this = Packet(temp);
}

Packet::Packet(char* dataIn)
{
	char indexChar[2] = { dataIn[0], dataIn[1] };
	char typeChar[2] = { dataIn[2], dataIn[3] };

	index = atoi(indexChar);
	type = (PacketType)atoi(typeChar);
	size = (PacketSize)((int)dataIn[3]);

	switch (size) {
	case SIZE_DEFAULT:
		buf = new char[DEFAULT_PACKET_SIZE];
		break;
	case SIZE_BIG:
		buf = new char[LARGER_PACKET_SIZE];
		break;
	case SIZE_MAXIMUM:
		buf = new char[MAX_PACKET_SIZE];
		break;
	}



	strcpy(buf, dataIn + 5);
}


Packet::~Packet()
{
	delete buf;
}

char* Packet::getPacketData()
{
	return nullptr;
}

int Packet::getPacketType()
{
	return 0;
}

int Packet::getUserIndex()
{
	return 0;
}

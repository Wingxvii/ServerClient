#include "ServerNetwork.h"

ServerNetwork::ServerNetwork()
{

}

ServerNetwork::~ServerNetwork()
{
}

// accept new connections
bool ServerNetwork::acceptNewClient(unsigned int & id)
{
	return false;
}

// receive incoming data
int ServerNetwork::receiveData(unsigned int client_id, char * recvbuf)
{
	return 0;
}

// send data to all clients
void ServerNetwork::sendToAll(char * packets, int totalSize)
{
}

void ServerNetwork::sendTo(char * packets, int totalSize, int clientID)
{
	
}

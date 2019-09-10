#include "ClientNetwork.h"

void listenToServer() {

}

ClientNetwork::ClientNetwork()
{
	//1: Start Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);

	//startup
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0) {
		cout << "cant start winsock" << wsOk;
		return;
	}

	//2. setup server information
	server.sin_family = AF_INET;
	server.sin_port = htons(54000);
	serverlength = sizeof(server);

	//3. setup socket
	client = socket(AF_INET, SOCK_DGRAM, 0);
}

ClientNetwork::~ClientNetwork()
{
	closesocket(client);
}

int ClientNetwork::connect()
{
	return connect(addressDefault);
}

int ClientNetwork::connect(string ip)
{
	inet_pton(AF_INET, ip.c_str(), &server.sin_addr);		//connecting to the server

	//ping and determine client index
	return 0;
}

int ClientNetwork::sendMessage(string message)
{
	message = clientIndex + message;
	int sendOK = sendto(client, message.c_str(), message.size() + 1, 0, (sockaddr*)& server, sizeof(server));
	if (sendOK == SOCKET_ERROR) {
		cout << "Send Error: " << WSAGetLastError() << endl;
		return -1;
	}
	return sendOK;
}

void ClientNetwork::startListening()
{
	/*
	std::thread thread_name_here(&ClientNetwork::sendMessage, this, "1278.0.1");
	thread_name_here.detach();
	*/

	thread listen = thread([&]() {
		while (true) {
			if (recvfrom(client, buf, 1024, 0, (sockaddr*)& server, &serverlength) == SOCKET_ERROR) {
				//cout << "Error reciving from server" << WSAGetLastError() << endl;
			}
			else {
				inQueue.push(buf);
			}
		}
		});
	listen.detach();
}



void ClientNetwork::setClientIndex(string index)
{
	clientIndex = index;
}

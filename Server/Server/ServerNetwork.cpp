#include "ServerNetwork.h"

ServerNetwork::ServerNetwork()
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
	//UDP does not require input, where TCP does
	in = socket(AF_INET, SOCK_DGRAM, 0);		//doesnt know when client disconnects	//make sure to send pings

	//initalization
	ConnectedUsers = std::vector<UserProfile>();
	inQueue = queue<char*>();


}

ServerNetwork::~ServerNetwork()
{
	closesocket(in);
}

// accept new connections
bool ServerNetwork::acceptNewClient()
{
	//bind socket to ip address and port
							//info about the client
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(54000); //convert from little to big endian
	clientLength = sizeof(serverHint);

	if (bind(in, (sockaddr*)& serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		cout << "Can't bind socket! " << WSAGetLastError() << endl;					//bind client info to client
	}

	return false;
}

void ServerNetwork::startListening()
{
	thread listen = thread([&]() {
		while (true) {
			if (recvfrom(in, buf, 1024, 0, (sockaddr*)& serverHint, &clientLength) == SOCKET_ERROR) {
				cout << "Error reciving from client" << WSAGetLastError() << endl;
			}
			else {
				inQueue.push(buf);
			}
		}
		});
	listen.detach();
}


// send data to all clients
void ServerNetwork::sendToAll(string message)
{


}

void ServerNetwork::sendTo(string message, int clientID)
{
	int sendOK = sendto(in, message.c_str(), message.size() + 1, 0, (sockaddr*)& serverHint, clientLength);
	if (sendOK == SOCKET_ERROR) {
		cout << "Send Error: " << WSAGetLastError() << endl;
	}

}

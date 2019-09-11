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

	//socket setup
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(54000); //convert from little to big endian
	clientLength = sizeof(serverHint);

	if (bind(in, (sockaddr*)& serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		cout << "Can't bind socket! " << WSAGetLastError() << endl;					//bind client info to client
	}

	//initalization
	ConnectedUsers = std::vector<UserProfile>();
	inQueue = vector<Packet>();
	
}

ServerNetwork::~ServerNetwork()
{
	closesocket(in);
}

// accept new connections
void ServerNetwork::acceptNewClient(sockaddr_in address, int length)
{
	UserProfile newProfile = UserProfile();

	newProfile.clientAddress = address;
	newProfile.clientLength = length;

	if (clientCount < 99) {
		clientCount++;
		newProfile.index = clientCount;
	}
	char str[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET, &(newProfile.clientAddress.sin_addr), str, INET_ADDRSTRLEN);
	newProfile.clientIP = str;

	ConnectedUsers.push_back(newProfile);
	//send acknowledgement message with user client index

	string acknowledgement = "00000";
	acknowledgement.append(to_string(newProfile.index));
	sendTo(acknowledgement, newProfile.index);

	cout << "Client Accepted" << endl;
}

void ServerNetwork::startListening()
{
	thread listen = thread([&]() {
		char* buf = new char[DEFAULT_PACKET_SIZE];

		while (true) {
			if (recvfrom(in, buf, DEFAULT_PACKET_SIZE, 0, (sockaddr*)& serverHint, &clientLength) == SOCKET_ERROR) {
				cout << "Error reciving from client" << WSAGetLastError() << endl;
			}
			else {
				Packet newPacket = Packet(buf);
				newPacket.source = serverHint;
				newPacket.sourceLength = clientLength;

				inQueue.push_back(newPacket);
			}

			for (Packet pack : inQueue) {
				//filters by type
				switch (pack.getPacketType()) {
				case PacketType::INIT_CONNECTION:	//init connection for a new client connection
					acceptNewClient(pack.source, pack.sourceLength);
					break;
				case PacketType::MESSAGE:			//string data 
					cout << "Message Recieved from " << to_string(ConnectedUsers[pack.getUserIndex()-1].index) << " : " << pack.getPacketData() << endl;
					break;
				}
			}
			//clears queue for next loop
			inQueue.clear();



		}
		});
	listen.detach();
}


// send data to all clients
void ServerNetwork::sendToAll(string message)
{
	for (UserProfile client : ConnectedUsers) {
		int sendOK = sendto(in, message.c_str(), DEFAULT_PACKET_SIZE, 0, (sockaddr*)& client.clientAddress, client.clientLength);
		if (sendOK == SOCKET_ERROR) {
			cout << "Send Error: " << WSAGetLastError() << endl;
		}
	}
}

void ServerNetwork::sendTo(string message, int clientID)
{
	int sendOK = sendto(in, message.c_str(), DEFAULT_PACKET_SIZE, 0, (sockaddr*)& ConnectedUsers[clientID-1].clientAddress, ConnectedUsers[clientID-1].clientLength);
	if (sendOK == SOCKET_ERROR) {
		cout << "Send Error: " << WSAGetLastError() << endl;
	}
}

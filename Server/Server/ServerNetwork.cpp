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
	serverHint.sin_port = htons(54222); //convert from little to big endian
	clientLength = sizeof(serverHint);

	if (bind(in, (sockaddr*)& serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		cout << "Can't bind socket! " << WSAGetLastError() << endl;					//bind client info to client
	}

	//initalization
	ConnectedUsers = std::vector<UserProfile>();
	packetsIn = vector<Packet>();
}

ServerNetwork::~ServerNetwork()
{
	listening = false;
	closesocket(in);
}

void ServerNetwork::acceptNewClient(std::vector<std::string> data, sockaddr_in address, int length)
{
	//processing must be done here
	int sender = std::stoi(data[0]);
	if (sender == 0) {
		UserProfile newProfile = UserProfile();
		newProfile.clientAddress = serverHint;
		newProfile.clientLength = clientLength;

		if (clientCount < 99) {
			clientCount++;
			newProfile.index = clientCount;
		}
		char str[INET6_ADDRSTRLEN];

		inet_ntop(AF_INET, &(newProfile.clientAddress.sin_addr), str, INET_ADDRSTRLEN);
		newProfile.clientIP = str;
		ConnectedUsers.push_back(newProfile);

		//send acknowledgement message with user client index
		Packet initPack;
		initPack.sender = 0;
		initPack.packet_type = INIT_CONNECTION;
		strcpy_s(initPack.data, (to_string(newProfile.index) + ",").c_str() + '\0');

		sendTo(initPack, newProfile.index);
		cout << "Client Accepted" << endl;
	}
	else {
		cout << "Client Already Connected";
	}
}

void ServerNetwork::startUpdates()
{
	thread listen = thread([&]() {
		char* buf = new char[MAX_PACKET_SIZE];

		while (listening) {
			int length = recvfrom(in, buf, MAX_PACKET_SIZE, 0, (sockaddr*)& serverHint, &clientLength);
			if (length == SOCKET_ERROR) {
				cout << "Recieve Error: " << WSAGetLastError() << endl;
			}

			if (length != SOCKET_ERROR) {
				Packet packet;
				std::vector<std::string> parsedData;

				int i = 0;
				while (i < (unsigned int)length) {
					packet.deserialize(&(buf[i]));
					i += sizeof(Packet);

					switch (packet.packet_type) {
					case PacketType::INIT_CONNECTION:

						//tokenize
						parsedData = Tokenizer::tokenize(',', packet.data);
						parsedData.insert(parsedData.begin(), to_string(packet.sender));

						acceptNewClient(parsedData, serverHint, clientLength);
						break;
					case PacketType::MESSAGE:
						//printOut(packet, packet.sender);
						relay(packet, packet.sender);
						break;
					case PacketType::PLAYERDATA:
						relay(packet, packet.sender);
						break;
					case PacketType::WEAPONSTATE:
						relay(packet, packet.sender);
						break;
					case PacketType::DAMAGEDEALT:
						//tokenize
						parsedData = Tokenizer::tokenize(',', packet.data);
						sendTo(packet, std::stoi(parsedData[0]));
						break;
					case PacketType::DROIDLOCATIONS:
						relay(packet, packet.sender);
						break;
					case PacketType::BUILD:
						relay(packet, packet.sender);
						break;
					case PacketType::KILL:
						relay(packet, packet.sender);
						break;
					case PacketType::GAMESTATE:
						relay(packet, packet.sender);
						break;

					default:
						break;
					}
				}
			}
		}
		});
	listen.detach();
}

void ServerNetwork::sendToAll(Packet pack)
{
	for (UserProfile client : ConnectedUsers) {
		const unsigned int packet_size = sizeof(pack);
		char packet_data[packet_size];

		pack.serialize(packet_data);

		int sendOK = sendto(in, packet_data, packet_size, 0, (sockaddr*)& client.clientAddress, client.clientLength);
		if (sendOK == SOCKET_ERROR) {
			cout << "Send Error: " << WSAGetLastError() << endl;
		}
	}
}

void ServerNetwork::sendTo(Packet pack, int clientID)
{
	const unsigned int packet_size = sizeof(pack);
	char packet_data[packet_size];

	pack.serialize(packet_data);

	int sendOK = sendto(in, packet_data, packet_size, 0, (sockaddr*)& ConnectedUsers[clientID - 1].clientAddress, ConnectedUsers[clientID - 1].clientLength);
	if (sendOK == SOCKET_ERROR) {
		cout << "Send Error: " << WSAGetLastError() << endl;
	}

}

void ServerNetwork::relay(Packet pack, int clientID)
{
	for (int counter = 0; counter < ConnectedUsers.size(); counter++) {
		/*
		if (counter + 1 == clientID) {
			continue;
		}
		*/
		const unsigned int packet_size = sizeof(pack);
		char packet_data[packet_size];

		pack.serialize(packet_data);

		int sendOK = sendto(in, packet_data, packet_size, 0, (sockaddr*)& ConnectedUsers[counter].clientAddress, ConnectedUsers[counter].clientLength);
		if (sendOK == SOCKET_ERROR) {
			cout << "Send Error: " << WSAGetLastError() << endl;
		}
	}

}

void ServerNetwork::printOut(Packet pack, int clientID)
{
	std::vector<std::string> parsedData;
	parsedData = Tokenizer::tokenize(',', pack.data);

	for (int i = 0; i < parsedData.size(); i++) {
		cout << parsedData[i] << endl;
	}
}

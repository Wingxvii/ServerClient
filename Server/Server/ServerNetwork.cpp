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

	//UDP socket connection
	udp = socket(AF_INET, SOCK_DGRAM, 0);
	//socket setup
	serverUDP.sin_addr.S_un.S_addr = ADDR_ANY;
	serverUDP.sin_family = AF_INET;
	serverUDP.sin_port = htons(54222);
	clientLength = sizeof(serverUDP);
	if (bind(udp, (sockaddr*)& serverUDP, sizeof(serverUDP)) == SOCKET_ERROR) {
		cout << "Can't bind socket! " << WSAGetLastError() << endl;					//bind client info to client
	}


	//TCP socket connection
	SOCKET tcp = socket(AF_INET, SOCK_DGRAM, 0);
	//socket setup
	serverTCP.sin_addr.S_un.S_addr = ADDR_ANY;
	serverTCP.sin_family = AF_INET;
	serverTCP.sin_port = htons(54223);
	if (bind(tcp, (sockaddr*)&serverTCP, sizeof(serverTCP)) == SOCKET_ERROR) {
		cout << "Can't bind socket! " << WSAGetLastError() << endl;					//bind client info to client
	}
	listen(tcp, SOMAXCONN);

	//zero the master list, then add in the listener socket
	FD_ZERO(&master);
	FD_SET(listening, &master);
	
	//initalization
	ConnectedUsers = std::vector<UserProfile>();
	packetsIn = vector<Packet>();
}

ServerNetwork::~ServerNetwork()
{
	listening = false;
	closesocket(udp);
}

void ServerNetwork::acceptNewClient(std::vector<std::string> data, sockaddr_in address, int length)
{
	//processing must be done here
	int sender = std::stoi(data[0]);
	if (sender != 0 && sender < ConnectedUsers.size()) {

		ConnectedUsers[sender].udpAddress = serverUDP;
		ConnectedUsers[sender].clientLength = clientLength;

		char str[INET6_ADDRSTRLEN];

		inet_ntop(AF_INET, &(ConnectedUsers[sender].udpAddress.sin_addr), str, INET_ADDRSTRLEN);
		ConnectedUsers[sender].clientIP = str;
		cout << "Client Accepted " << ConnectedUsers[sender].index << endl;
	}
	else {
		cout << "Connection Error";
	}
}

void ServerNetwork::startUpdates()
{
	cout << "Server Running..." << endl;

	thread udpUpdate = thread([&]() {

		char* buf = new char[DEFAULT_DATA_SIZE];
		ZeroMemory(buf, DEFAULT_DATA_SIZE);

		while (listening) {
			int length = recvfrom(udp, buf, DEFAULT_DATA_SIZE, 0, (sockaddr*)& serverUDP, &clientLength);
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

						acceptNewClient(parsedData, serverUDP, clientLength);
						break;
					case PacketType::MESSAGE:
						//printOut(packet, packet.sender);
						relay(packet, packet.sender);
						break;

					default:
						break;
					}
				}
			}
		}
		});
	udpUpdate.detach();

	thread tcpUpdate = thread([&]() {
		while (listening) {

		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			//create new client profile
			if (sock == listening)
			{
				// Accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				//create new profile
				UserProfile newProfile = UserProfile();
				newProfile.tcpSocket = client;

				if (clientCount < 99) {
					clientCount++;
					newProfile.index = clientCount;
				}

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);
				ConnectedUsers.push_back(newProfile);

				//send outgoing connection packet back to client
				Packet initPack;
				initPack.sender = 0;
				initPack.packet_type = INIT_CONNECTION;
				strcpy_s(initPack.data, (to_string(newProfile.index) + ",").c_str() + '\0');

				sendTo(initPack, client);
			}
			else {
				char* buf = new char[DEFAULT_DATA_SIZE];
				ZeroMemory(buf, DEFAULT_DATA_SIZE);

				int length = recv(sock, buf, DEFAULT_DATA_SIZE, 0);

				if (length == SOCKET_ERROR) {
					cout << "Recieve Error: " << WSAGetLastError() << endl;
				}
				else {
					Packet packet;
					std::vector<std::string> parsedData;

					int i = 0;
					while (i < (unsigned int)length) {
						packet.deserialize(&(buf[i]));
						i += sizeof(Packet);

						switch (packet.packet_type) {
						case PacketType::INIT_CONNECTION:
							cout << "Error: Incomming connection packet through invalid TCP channels";
							break;
						case PacketType::MESSAGE:
							relay(packet, packet.sender);
							break;
						default:
							break;
						}
					}
				}



			}
		}

		}
		});
	tcpUpdate.detach();

}

void ServerNetwork::sendToAll(Packet pack)
{
	for (UserProfile client : ConnectedUsers) {
		const unsigned int packet_size = sizeof(pack);
		char packet_data[packet_size];

		pack.serialize(packet_data);

		int sendOK = sendto(udp, packet_data, packet_size, 0, (sockaddr*)& client.udpAddress, client.clientLength);
		if (sendOK == SOCKET_ERROR) {
			cout << "Send Error: " << WSAGetLastError() << endl;
		}
	}
}

//the UDP Send
void ServerNetwork::sendTo(Packet pack, int clientID)
{
	const unsigned int packet_size = sizeof(pack);
	char packet_data[packet_size];

	pack.serialize(packet_data);

	int sendOK = sendto(udp, packet_data, packet_size, 0, (sockaddr*)& ConnectedUsers[clientID - 1].udpAddress, ConnectedUsers[clientID - 1].clientLength);
	if (sendOK == SOCKET_ERROR) {
		cout << "Send Error: " << WSAGetLastError() << endl;
	}
	
}


//the TCP Send
void ServerNetwork::sendTo(Packet pack, SOCKET client)
{
	const unsigned int packet_size = sizeof(pack);
	char packet_data[packet_size];


	pack.serialize(packet_data);
	int sendOK = send(client, packet_data, packet_size, 0);
	if (sendOK == SOCKET_ERROR) {
		cout << "Send Error: " << WSAGetLastError() << endl;
	}
}

void ServerNetwork::relay(Packet pack, int clientID, bool useTCP)
{
	for (int counter = 0; counter < ConnectedUsers.size(); counter++) {
		
		if (counter + 1 == clientID) {
			continue;
		}
		const unsigned int packet_size = sizeof(pack);
		char packet_data[packet_size];

		pack.serialize(packet_data);

		if (!useTCP) {

			int sendOK = sendto(udp, packet_data, packet_size, 0, (sockaddr*)&ConnectedUsers[counter].udpAddress, ConnectedUsers[counter].clientLength);
			if (sendOK == SOCKET_ERROR) {
				cout << "Send Error: " << WSAGetLastError() << endl;
			}
		}
		else {
			int sendOK = send(ConnectedUsers[counter].tcpSocket, packet_data, packet_size, 0);
			if (sendOK == SOCKET_ERROR) {
				cout << "Send Error: " << WSAGetLastError() << endl;
			}
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



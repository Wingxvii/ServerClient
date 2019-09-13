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
	messagesIn = vector<std::vector<std::string>>();
}

ServerNetwork::~ServerNetwork()
{
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
		sendTo(INIT_CONNECTION, to_string(newProfile.index), newProfile.index);
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

		while (true) {
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
					parsedData = Tokenizer::tokenize(',', packet.data);
					parsedData.insert(parsedData.begin(), to_string(packet.sender));

					switch (packet.packet_type) {
					case PacketType::INIT_CONNECTION:
						acceptNewClient(parsedData, serverHint, clientLength);
						break;
					case PacketType::MESSAGE:
						messagesIn.push_back(parsedData);
						break;
					default:
						break;
					}
				}
			}

			//process messages
			for (std::vector<std::string> parsedData : messagesIn) {
				int sender = std::stoi(parsedData[0]);

				//filter by sender
				if (sender == 0) {
					string message = "";
					for (int counter = 1; counter < parsedData.size(); counter++) {
						message = message + parsedData[counter];
					}
					cout << "Message Recieved from Server :" << message << endl;
				}
				else {
					string message = "";
					for (int counter = 1; counter < parsedData.size(); counter++) {
						message = message + parsedData[counter];
					}
					cout << "Message Recieved from Client " << parsedData[0] << " :" << message << endl;
					
					//relay
					relay(MESSAGE, message, sender);
				}
			}
			messagesIn.clear();

		}
		});
	listen.detach();
}


// send data to all clients
void ServerNetwork::sendToAll(int packetType, string message)
{
	//include for tokenizer
	message = message + ",";

	for (UserProfile client : ConnectedUsers) {
		Packet packet;
		strcpy_s(packet.data, message.c_str() + '\0');
		packet.packet_type = packetType;
		packet.sender = 0;

		const unsigned int packet_size = sizeof(packet);
		char packet_data[packet_size];

		packet.serialize(packet_data);

		int sendOK = sendto(in, packet_data, packet_size, 0, (sockaddr*)& client.clientAddress, client.clientLength);
		if (sendOK == SOCKET_ERROR) {
			cout << "Send Error: " << WSAGetLastError() << endl;
		}
	}
}

void ServerNetwork::sendTo(int packetType, string message, int clientID)
{
	//include for tokenizer
	message = message + ",";

	Packet packet;
	strcpy_s(packet.data, message.c_str() + '\0');
	packet.packet_type = packetType;
	packet.sender = 0;

	const unsigned int packet_size = sizeof(packet);
	char packet_data[packet_size];

	packet.serialize(packet_data);

	int sendOK = sendto(in, packet_data, packet_size, 0, (sockaddr*)& ConnectedUsers[clientID - 1].clientAddress, ConnectedUsers[clientID - 1].clientLength);
	if (sendOK == SOCKET_ERROR) {
		cout << "Send Error: " << WSAGetLastError() << endl;
	}
}

void ServerNetwork::relay(int packetType, string message, int clientID)
{
	//include for tokenizer
	message = message + ",";

	for (int counter = 0; counter < ConnectedUsers.size();counter++ ) {
		if (counter+1 == clientID) {
			continue;
		}
		Packet packet;
		strcpy_s(packet.data, message.c_str() + '\0');
		packet.packet_type = packetType;
		packet.sender = clientID;

		const unsigned int packet_size = sizeof(packet);
		char packet_data[packet_size];

		packet.serialize(packet_data);

		int sendOK = sendto(in, packet_data, packet_size, 0, (sockaddr*)& ConnectedUsers[counter].clientAddress, ConnectedUsers[counter].clientLength);
		if (sendOK == SOCKET_ERROR) {
			cout << "Send Error: " << WSAGetLastError() << endl;
		}
	}
}

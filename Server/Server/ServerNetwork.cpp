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
	if (bind(udp, (sockaddr*)&serverUDP, sizeof(serverUDP)) == SOCKET_ERROR) {
		cout << "Can't bind socket! " << WSAGetLastError() << endl;					//bind client info to client
	}

	//TCP socket connection
	tcp = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp == INVALID_SOCKET)
	{
		cerr << "Can't create a socket!" << endl;
	}

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
	FD_SET(tcp, &master);
	FD_SET(udp, &master);

	//initalization
	ConnectedUsers = std::vector<UserProfile>();
	packetsIn = vector<Packet>();


}

ServerNetwork::~ServerNetwork()
{
	listening = false;
	FD_CLR(tcp, &master);
	closesocket(tcp);
	closesocket(udp);
	WSACleanup();

}

void ServerNetwork::acceptNewClient(std::vector<std::string> data, sockaddr_in address, int length)
{
	//processing must be done here
	int sender = std::stoi(data[0]);
	if (sender < ConnectedUsers.size()) {

		ConnectedUsers[sender].udpAddress = serverUDP;
		ConnectedUsers[sender].clientLength = clientLength;
		ConnectedUsers[sender].active = true;

		char str[INET6_ADDRSTRLEN];

		inet_ntop(AF_INET, &(ConnectedUsers[sender].udpAddress.sin_addr), str, INET_ADDRSTRLEN);
		ConnectedUsers[sender].clientIP = str;
		cout << "Client " << ConnectedUsers[sender].index << " has connected." << endl;


	}
	else {
		cout << "Connection Error";
	}
}

void ServerNetwork::startUpdates()
{
	
	cout << "Server Running..." << endl;
	thread CommandLine = thread([&]() {

		while (listening) {
			string command;
			std::getline(std::cin, command);
			if (command == "/quit") {
				listening = false;
			}

			if (command == "/open") {
				system("START C:\\Users\\turnt\\OneDrive\\Documents\\C++\\ServerClient\\Client\\x64\\Debug\\client.exe");
			}

			if (command == "/clear") {
			
				ConnectedUsers.clear();
				clientCount = 0;
				FD_ZERO(&master);
				FD_SET(tcp, &master);
				FD_SET(udp, &master);
				cout << "Cleared User Cache" << endl;
			}

			if (command == "/testUDP" || command == "/test") {
				Packet pack;
				pack.packet_type = MESSAGE;

				string test = "sending UDP packet";

				strcpy_s(pack.data, test.c_str() + '\0');
				pack.sender = -1;
				relay(pack, false);
			}
			if (command == "/testTCP") {
				Packet pack;
				pack.packet_type = MESSAGE;

				string test = "sending TCP packet";

				strcpy_s(pack.data, test.c_str() + '\0');
				pack.sender = -1;
				relay(pack, true);
			}
			if (command == "/help") {
				cout << "/quit - quits application" << endl;
				cout << "/open - starts a independant client" << endl;
				cout << "/clear - clears user cache" << endl;
				cout << "/testUDP - Pings a UDP packet" << endl;
				cout << "/testTCP - Pings a TCP packet" << endl;

			}
			else {
				cout << "Command unknown. Input /help for help.";
			}

		}

	});
	CommandLine.detach();
	

	while (listening) {

		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		//handle all active sockets
		for (int i = 0; i < socketCount; i++)
		{
			cout << "Packet Recieved:";

			SOCKET sock = copy.fd_array[i];

			//TCP New Connection Listener
			if (sock == tcp)
			{
				cout << "TCP Connection" << endl;

				// Accept a new connection
				SOCKET client = accept(tcp, nullptr, nullptr);

				//create new profile
				UserProfile newProfile = UserProfile();
				newProfile.tcpSocket = client;

				if (clientCount < 99) {
					newProfile.index = clientCount;
					clientCount++;
				}

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);
				ConnectedUsers.push_back(newProfile);


				//send outgoing connection packet back to client
				Packet initPack;
				initPack.sender = -1;
				initPack.packet_type = INIT_CONNECTION;
				strcpy_s(initPack.data, (to_string(newProfile.index) + ",").c_str() + '\0');
				sendTo(initPack, client);

			}
			//UDP Input Socket
			else if (sock == udp) {
				char* buf = new char[MAX_PACKET_SIZE];

				//recieve packet from socket
				int length = recvfrom(udp, buf, MAX_PACKET_SIZE, 0, (sockaddr*)&serverUDP, &clientLength);
				if (length == SOCKET_ERROR) {
					cout << "UDP Recieve Error: " << WSAGetLastError() << endl;
				}
				else {
					//deseralize socket data into packet
					Packet packet;
					packet.deserialize(buf);

					//process connection packet
					if (packet.packet_type == PacketType::INIT_CONNECTION) {
						cout << "UDP Connection" << endl;
						std::vector<std::string> parsedData;

						//tokenize
						parsedData = Tokenizer::tokenize(',', packet.data);
						parsedData.insert(parsedData.begin(), to_string(packet.sender));

						acceptNewClient(parsedData, serverUDP, clientLength);
						break;
					}
					else {
						//process if not connection
						ProcessUDP(packet);
					}
				}
			}
			//TCP Input Sockets
			else {
				char* buf = new char[MAX_PACKET_SIZE];

				int length = recv(sock, buf, MAX_PACKET_SIZE, 0);
				//handles disconnection packets
				if (length <= 0)
				{
					cout << "Disconnection" << endl;

					//handle disconnected user updates
					for (UserProfile profile : ConnectedUsers) {
						if (sock == profile.tcpSocket) {
							profile.active = false;
						}
					}

					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else if (length == SOCKET_ERROR) {
					cout << "Recieve Error: " << WSAGetLastError() << endl;
				}
				else {
					//process packet data
					Packet packet;
					packet.deserialize(buf);
					ProcessTCP(packet);

				}
			}
		}
	}
}

void ServerNetwork::sendToAll(Packet pack)
{
	for (UserProfile client : ConnectedUsers) {
		const unsigned int packet_size = sizeof(pack);
		char packet_data[packet_size];

		pack.serialize(packet_data);

		int sendOK = sendto(udp, packet_data, packet_size, 0, (sockaddr*)&client.udpAddress, client.clientLength);
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

	int sendOK = sendto(udp, packet_data, packet_size, 0, (sockaddr*)&ConnectedUsers[clientID - 1].udpAddress, ConnectedUsers[clientID - 1].clientLength);
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

//processes all TCP Packets
void ServerNetwork::ProcessTCP(Packet pack)
{
	std::vector<std::string> parsedData;

	//packet processing 
	switch (pack.packet_type) {
	case PacketType::INIT_CONNECTION:
		cout << "Data Pipeline Error: INIT Connection not handled correctly";
		break;

	//relay the data
	case PacketType::MESSAGE:
		parsedData = Tokenizer::tokenize(',', pack.data);
		cout << "TCP Message Recieved from user (" + to_string(pack.sender) + "):" << parsedData[0] << endl;

		relay(pack, true);

		break;

	case PacketType::WEAPON_DATA:
	case PacketType::BUILD_ENTITY:
	case PacketType::KILL_ENTITY:
	case PacketType::GAME_STATE:
		relay(pack, true);
		break;
	
	//send the data to RTS player
	case PacketType::ENVIRONMENT_DAMAGE:
		sendTo(pack, ConnectedUsers[0].tcpSocket);
		break;

	//send the data to sepific player
	case PacketType::PLAYER_DAMAGE:
		parsedData = Tokenizer::tokenize(',', pack.data);
		sendTo(pack, stoi(parsedData[0]));
		break;

	case PacketType::PLAYER_DATA:
		cout << "Data Protocol Use Invalid: PLAYER_DATA:TCP";
		break;
	case PacketType::DROID_POSITION:
		cout << "Data Protocol Use Invalid: DROID_POSITION:TCP";
		break;
	case PacketType::TURRET_DATA:
		cout << "Data Protocol Use Invalid: TURRET_DATA:TCP";
		break;
	default:
		cout << "Error: Unhandled Packet Type";
		break;
	}

}

//processes all UDP Packets
void ServerNetwork::ProcessUDP(Packet pack)
{
	std::vector<std::string> parsedData;

	switch (pack.packet_type) {

	//relay the data to all clients
	case PacketType::MESSAGE:
		parsedData = Tokenizer::tokenize(',', pack.data);

		cout << "UDP Message Recieved from user (" + to_string(pack.sender) + "):" << parsedData[0] << endl;
		relay(pack);

		break;
	case PacketType::PLAYER_DATA:
	case PacketType::DROID_POSITION:
	case PacketType::TURRET_DATA:
		relay(pack, false);
		break;


	case PacketType::INIT_CONNECTION:
		cout << "Error: Incomming connection packet through invalid TCP channels";
		break;
	case PacketType::WEAPON_DATA:
		cout << "Data Protocol Use Invalid: WEAPON_DATA:UDP";
		break;
	case PacketType::ENVIRONMENT_DAMAGE:
		cout << "Data Protocol Use Invalid: ENVIRONMENT_DAMAGE:UDP";
		break;
	case PacketType::BUILD_ENTITY:
		cout << "Data Protocol Use Invalid: BUILD_ENTITY:UDP";
		break;
	case PacketType::KILL_ENTITY:
		cout << "Data Protocol Use Invalid: KILL_ENTITY:UDP";
		break;
	case PacketType::GAME_STATE:
		cout << "Data Protocol Use Invalid: GAME_STATE:UDP";
		break;
	case PacketType::PLAYER_DAMAGE:
		cout << "Data Protocol Use Invalid: PLAYER_DAMAGE:UDP";
		break;
	default:
		cout << "Error: Unhandled Packet Type";
		break;
	}
}

void ServerNetwork::relay(Packet pack, bool useTCP)
{
	for (int counter = 0; counter < ConnectedUsers.size(); counter++) {

		if (counter == pack.sender) {
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



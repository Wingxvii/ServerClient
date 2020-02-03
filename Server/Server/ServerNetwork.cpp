#include "ServerNetwork.h"
ServerNetwork::ServerNetwork()
{
	//1: Start Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);

	//startup
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0) {
		std::cout << "cant start winsock" << wsOk;
		return;
	}

	//UDP socket connection
	//socket setup
	serverUDP.sin_addr.S_un.S_addr = ADDR_ANY;
	serverUDP.sin_family = AF_INET;
	serverUDP.sin_port = htons(54222);
	clientLength = sizeof(serverUDP);
	udp = socket(serverUDP.sin_family, SOCK_DGRAM, IPPROTO_UDP);
	if (bind(udp, (const sockaddr*)&serverUDP, sizeof(serverUDP)) == SOCKET_ERROR) {
		std::cout << "Can't bind socket! " << WSAGetLastError() << std::endl;					//bind client info to client
	}

	//TCP socket connection
	//socket setup
	serverTCP.sin_addr.S_un.S_addr = ADDR_ANY;
	serverTCP.sin_family = AF_INET;
	serverTCP.sin_port = htons(54223);
	tcp = socket(serverTCP.sin_family, SOCK_STREAM, IPPROTO_TCP);
	if (tcp == INVALID_SOCKET)
	{
		std::cout << "Can't create a socket!" << std::endl;
	}

	if (bind(tcp, (sockaddr*)&serverTCP, sizeof(serverTCP)) == SOCKET_ERROR) {
		std::cout << "Can't bind socket! " << WSAGetLastError() << std::endl;					//bind client info to client
	}
	listen(tcp, SOMAXCONN);

	//zero the master list, then add in the listener socket
	FD_ZERO(&master);
	FD_SET(tcp, &master);
	FD_SET(udp, &master);

	//initalization
	ConnectedUsers = std::vector<UserProfile>();
	packetsIn = std::vector<Packet>();

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
		std::cout << "Client " << ConnectedUsers[sender].index << " has connected." << std::endl;


	}
	else {
		std::cout << "Connection Error";
	}
}

void ServerNetwork::startUpdates()
{

	std::cout << "Server Running..." << std::endl;
	std::thread CommandLine = std::thread([&]() {

		while (listening) {
			std::string command;
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
				std::cout << "Cleared User Cache" << std::endl;
			}

			if (command == "/testUDP" || command == "/test") {
				Packet pack;
				pack.packet_type = MESSAGE;

				std::string test = "sending UDP packet";

				strcpy_s(pack.data, test.c_str() + '\0');
				pack.sender = -1;
				relay(pack, false);
			}
			if (command == "/testTCP") {
				Packet pack;
				pack.packet_type = MESSAGE;

				std::string test = "sending TCP packet";

				strcpy_s(pack.data, test.c_str() + '\0');
				pack.sender = -1;
				relay(pack, true);
			}
			if (command == "/help") {
				std::cout << "/quit - quits application" << std::endl;
				std::cout << "/open - starts a independant client" << std::endl;
				std::cout << "/clear - clears user cache" << std::endl;
				std::cout << "/testUDP - Pings a UDP packet" << std::endl;
				std::cout << "/testTCP - Pings a TCP packet" << std::endl;

			}
			else {
				std::cout << "Command unknown. Input /help for help.";
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
			// std::cout << "Packet Recieved:";

			SOCKET sock = copy.fd_array[i];

			//TCP New Connection Listener
			if (sock == tcp)
			{
				std::cout << "TCP Connection" << std::endl;

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
				strcpy_s(initPack.data, (std::to_string(newProfile.index) + ",").c_str() + '\0');
				sendTo(initPack, client);

			}
			//UDP Input Socket
			else if (sock == udp) {
				char* buf = new char[MAX_PACKET_SIZE];

				//recieve packet from socket
				int length = recvfrom(udp, buf, MAX_PACKET_SIZE, 0, (sockaddr*)&serverUDP, &clientLength);
				if (length == SOCKET_ERROR) {
					std::cout << "UDP Recieve Error: " << WSAGetLastError() << std::endl;
				}
				else {
					//deseralize socket data into packet
					Packet packet;
					packet.deserialize(buf);

					//process connection packet
					if (packet.packet_type == PacketType::INIT_CONNECTION) {
						std::cout << "UDP Connection" << std::endl;
						std::vector<std::string> parsedData;

						//tokenize
						parsedData = Tokenizer::tokenize(',', packet.data);
						parsedData.insert(parsedData.begin(), std::to_string(packet.sender));

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
					std::cout << "Disconnection" << std::endl;

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
					std::cout << "Recieve Error: " << WSAGetLastError() << std::endl;
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
			std::cout << "Send Error: " << WSAGetLastError() << std::endl;
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
		std::cout << "Send Error: " << WSAGetLastError() << std::endl;
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
		std::cout << "Send Error: " << WSAGetLastError() << std::endl;
	}
}

//processes all TCP Packets
void ServerNetwork::ProcessTCP(Packet pack)
{
	std::vector<std::string> parsedData;

	//packet processing 
	switch (pack.packet_type)
	{
	case PacketType::INIT_CONNECTION:
		std::cout << "Data Pipeline Error: INIT Connection not handled correctly";
		break;

		//relay the data
	case PacketType::MESSAGE:
		parsedData = Tokenizer::tokenize(',', pack.data);
		std::cout << "TCP Message Recieved from user (" + std::to_string(pack.sender) + "):" << parsedData[0] << std::endl;

		relay(pack, true);

		break;

	case PacketType::WEAPON_DATA:
	case PacketType::BUILD_ENTITY:
	case PacketType::KILL_ENTITY:
	case PacketType::GAME_STATE:
		std::cout << "TCP Packet Type: " << pack.packet_type << " , ID: " << pack.id << std::endl;
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
		std::cout << "Data Protocol Use Invalid: PLAYER_DATA:TCP";
		break;
	case PacketType::DROID_POSITION:
		std::cout << "Data Protocol Use Invalid: DROID_POSITION:TCP";
		break;
	case PacketType::TURRET_DATA:
		std::cout << "Data Protocol Use Invalid: TURRET_DATA:TCP";
		break;
	default:
		std::cout << "Error: TCP Unhandled Packet Type: " << pack.packet_type << std::endl;
		break;
	}

}

//processes all UDP Packets
void ServerNetwork::ProcessUDP(Packet pack)
{
	std::vector<std::string> parsedData;

	switch (pack.packet_type)
	{
		//relay the data to all clients
	case PacketType::MESSAGE:
		parsedData = Tokenizer::tokenize(',', pack.data);

		std::cout << "UDP Message Recieved from user (" + std::to_string(pack.sender) + "):" << parsedData[0] << std::endl;
		relay(pack);

		break;
	case PacketType::PLAYER_DATA:
	case PacketType::DROID_POSITION:
	case PacketType::TURRET_DATA:
		relay(pack, false);
		break;


	case PacketType::INIT_CONNECTION:
		std::cout << "Error: Incomming connection packet through invalid TCP channels";
		break;
	case PacketType::WEAPON_DATA:
		std::cout << "Data Protocol Use Invalid: WEAPON_DATA:UDP";
		break;
	case PacketType::ENVIRONMENT_DAMAGE:
		std::cout << "Data Protocol Use Invalid: ENVIRONMENT_DAMAGE:UDP";
		break;
	case PacketType::BUILD_ENTITY:
		std::cout << "Data Protocol Use Invalid: BUILD_ENTITY:UDP";
		break;
	case PacketType::KILL_ENTITY:
		std::cout << "Data Protocol Use Invalid: KILL_ENTITY:UDP";
		break;
	case PacketType::GAME_STATE:
		std::cout << "Data Protocol Use Invalid: GAME_STATE:UDP";
		break;
	case PacketType::PLAYER_DAMAGE:
		std::cout << "Data Protocol Use Invalid: PLAYER_DAMAGE:UDP";
		break;
	default:
		std::cout << "Error: UDP Unhandled Packet Type:" << pack.packet_type << std::endl;
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
				std::cout << "Send Error: " << WSAGetLastError() << std::endl;
			}
		}
		else {
			int sendOK = send(ConnectedUsers[counter].tcpSocket, packet_data, packet_size, 0);
			if (sendOK == SOCKET_ERROR) {
				std::cout << "Send Error: " << WSAGetLastError() << std::endl;
			}
		}
	}
}

void ServerNetwork::printOut(Packet pack, int clientID)
{
	std::vector<std::string> parsedData;
	parsedData = Tokenizer::tokenize(',', pack.data);

	for (int i = 0; i < parsedData.size(); i++) {
		std::cout << parsedData[i] << std::endl;
	}
}



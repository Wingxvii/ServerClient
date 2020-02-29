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
		ConnectedUsers[sender].Username = data[1];

		char str[INET6_ADDRSTRLEN];

		inet_ntop(AF_INET, &(ConnectedUsers[sender].udpAddress.sin_addr), str, INET_ADDRSTRLEN);
		ConnectedUsers[sender].clientIP = str;
		cout << "Client " << ConnectedUsers[sender].Username << " has connected." << endl;

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
			//cout << "Packet Recieved:";

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

						acceptNewClient(parsedData, serverUDP, clientLength);
						sendTo(createPacket(INIT_CONNECTION, to_string(packet.sender), -1), packet.sender);
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

	int sendOK = sendto(udp, packet_data, packet_size, 0, (sockaddr*)&ConnectedUsers[clientID].udpAddress, ConnectedUsers[clientID].clientLength);
	if (sendOK == SOCKET_ERROR) {
		cout << "Send Error: " << WSAGetLastError() << endl;
	}

}

Packet ServerNetwork::createPacket(PacketType type, string data, int sender)
{
	Packet newPacket = Packet();
	newPacket.packet_type = type;
	strcpy_s(newPacket.data, (data).c_str() + '\0');
	newPacket.sender = sender;

	return newPacket;
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
	parsedData = Tokenizer::tokenize(',', pack.data);

	string outData = "";


	//packet processing 
	switch (pack.packet_type) {
	case PacketType::INIT_CONNECTION:
		cout << "Data Pipeline Error: INIT Connection not handled correctly";
		break;

	//relay the data
	case PacketType::MESSAGE:
		cout << "TCP Message Recieved from user (" + ConnectedUsers[pack.sender].Username + "):" << parsedData[0] << endl;

		//add username to message
		strcpy_s(pack.data, (parsedData[0] + "," + ConnectedUsers[pack.sender].Username).c_str() + '\0');

		//send message to all in game
		if (ConnectedUsers[pack.sender].inGame) {
			for (int index : ActiveGames[ConnectedUsers[pack.sender].gameNumber]) {
				sendTo(pack, ConnectedUsers[index].tcpSocket);
			}
		}
		else {
			//SEND ERROR PACKET
		}
		break;

	case PacketType::REQUEST_GAME:
		//send a request game packet to target
		sendTo(createPacket(REQUEST_GAME, ConnectedUsers[pack.sender].Username, pack.sender), ConnectedUsers[stoi(parsedData[0])].tcpSocket );

		break;
	
	case PacketType::REQUEST_RESPONSE:
		//send response to target
		sendTo(createPacket(REQUEST_RESPONSE, parsedData[1] + "," + ConnectedUsers[stoi(parsedData[0])].Username, pack.sender), ConnectedUsers[stoi(parsedData[0])].tcpSocket);
		
		//enter the requester into a game with the sender
		if (parsedData[0] == "1") {
			JoinGame(stoi(parsedData[0]), pack.sender);
		}

		break;
	case PacketType::GAME_QUIT:
		if (ConnectedUsers[pack.sender].inGame) {
			ConnectedUsers[pack.sender].inGame = false;

			//remove user from list
			for (int counter = 0; counter < ActiveGames[ConnectedUsers[pack.sender].gameNumber].size(); counter++) {
				if (ActiveGames[ConnectedUsers[pack.sender].gameNumber][counter] == pack.sender) {
					ActiveGames[ConnectedUsers[pack.sender].gameNumber].erase(ActiveGames[ConnectedUsers[pack.sender].gameNumber].begin() + counter);
				}
			}
			
			//remove list if empty
			if (ActiveGames[ConnectedUsers[pack.sender].gameNumber].empty()) {
				ActiveGames.erase(ActiveGames.begin() + ConnectedUsers[pack.sender].gameNumber);
			}
		}
		break;

	case PacketType::SESSION_DATA:


		for (int user : ActiveGames[ConnectedUsers[pack.sender].gameNumber]) {
			//user index
			outData += to_string(ConnectedUsers[user].index);
			outData += ",";

			//username
			outData += ConnectedUsers[user].Username;
			outData += ",";

			//is busy
			outData += to_string(1);
			outData += ",";
		}

		sendTo(createPacket(LOBBY_DATA, outData, -1), ConnectedUsers[pack.sender].tcpSocket);


		break;

	case PacketType::LOBBY_DATA:

		for (UserProfile user: ConnectedUsers) {
			//user index
			outData += to_string(user.index);
			outData += ",";

			//username
			outData += user.Username;
			outData += ",";

			//is busy
			outData += to_string(user.inGame);
			outData += ",";
		}

		sendTo(createPacket(LOBBY_DATA, outData, -1), ConnectedUsers[pack.sender].tcpSocket);

		break;

	default:
		//cout << "Error: Unhandled Packet Type: TCP " << to_string(pack.packet_type) << endl;
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

		cout << "UDP Message Recieved from user (" + ConnectedUsers[pack.sender].Username + "):" << parsedData[0] << endl;
		//add username to message
		strcpy_s(pack.data, (parsedData[0] + "," + ConnectedUsers[pack.sender].Username).c_str() + '\0');

		relay(pack);

		break;
	default:
		//cout << "Error: Unhandled Packet Type: UDP " << to_string(pack.packet_type) <<  endl;
		break;
	}
}

void ServerNetwork::JoinGame(int requester, int responder)
{
	//if responder is in game, add requester to game
	if (ConnectedUsers[responder].inGame) {
		ConnectedUsers[requester].inGame = true;

		//find game that responder is in
		for (vector<int> game : ActiveGames) {
			for (int counter = 0; counter < game.size(); counter++) {
				//if found, add requester
				if (game[counter] == responder) {
					game.push_back(requester);
					break;
				}
			}
		}
	}
	//if responder is not in game, create new game
	else {
		//create new game
		vector<int> newGame = { responder, requester };
		ConnectedUsers[responder].gameNumber = ActiveGames.size();
		ConnectedUsers[requester].gameNumber = ActiveGames.size();

		//add to game to list
		ActiveGames.push_back(newGame);
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



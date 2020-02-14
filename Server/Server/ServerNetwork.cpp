#include "ServerNetwork.h"

ServerNetwork::ServerNetwork()
{
	previousTime = std::chrono::system_clock::now();
	//1: Start Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);

	//startup
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0) {
		std::cout << "cant start winsock" << wsOk;
		return;
	}

	//TCP socket connection
	//socket setup
	serverTCP.sin_addr.S_un.S_addr = ADDR_ANY;
	serverTCP.sin_family = AF_INET;
	serverTCP.sin_port = htons(5000);
	tcp = socket(serverTCP.sin_family, SOCK_STREAM, IPPROTO_TCP);
	if (tcp == INVALID_SOCKET)
	{
		std::cout << "Can't create a socket! " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	if (bind(tcp, (sockaddr*)&serverTCP, sizeof(serverTCP)) == SOCKET_ERROR) {
		std::cout << "Can't bind socket! " << WSAGetLastError() << std::endl;					//bind client info to client
		closesocket(tcp);
		WSACleanup();
		return;
	}

	//UDP socket connection
	//socket setup
	serverUDP.sin_addr.S_un.S_addr = ADDR_ANY;
	serverUDP.sin_family = AF_INET;
	serverUDP.sin_port = htons(5001);
	udp = socket(serverUDP.sin_family, SOCK_DGRAM, IPPROTO_UDP);
	if (udp == INVALID_SOCKET)
	{
		std::cout << "Can't create a socket! " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	if (bind(udp, (sockaddr*)&serverUDP, sizeof(serverUDP)) == SOCKET_ERROR) {
		std::cout << "Can't bind socket! " << WSAGetLastError() << std::endl;					//bind client info to client
		closesocket(udp);
		WSACleanup();
		return;
	}

	listen(tcp, SOMAXCONN);

	//zero the master list, then add in the listener socket
	FD_ZERO(&master);
	FD_SET(tcp, &master);
	FD_SET(udp, &master);

	//initalization
	ConnectedUsers = std::vector<UserProfile>();
	//packetsIn = std::vector<Packet>();
}

ServerNetwork::~ServerNetwork()
{
	listening = false;
	FD_CLR(tcp, &master);
	closesocket(tcp);
	closesocket(udp);
	WSACleanup();
}

void ServerNetwork::acceptNewClient(int sender, sockaddr_in address, int length)
{
	//processing must be done here
	if (sender < ConnectedUsers.size()) {
		if (address.sin_addr.S_un.S_addr != ConnectedUsers[sender].udpAddress.sin_addr.S_un.S_addr)
		{
			ConnectedUsers[sender].udpAddress = address;
			ConnectedUsers[sender].clientLength = length;

			//char str[INET6_ADDRSTRLEN];
			//
			//inet_ntop(AF_INET, &(ConnectedUsers[sender].udpAddress.sin_addr), str, INET_ADDRSTRLEN);
			//ConnectedUsers[sender].clientIP = str;

			if (!ConnectedUsers[sender].activeUDP)
			{
				std::cout << "UDP Client " << ConnectedUsers[sender].index << " has connected." << std::endl;
				ConnectedUsers[sender].activeUDP = true;
			}
		}
	}
	else {
		std::cout << "Connection Error";
	}
}

void ServerNetwork::SetReady(bool readyState)
{
	if (readyState)
	{
		allReady = true;
		// Check all other readys
		for (int i = 0; i < clientCount; i++)
		{
			if (!ConnectedUsers[i].ready)
			{
				allReady = false;
				break;
			}
		}
		if (allReady)
		{
			char packetData[DEFAULT_DATA_SIZE];
			int loc = INITIAL_OFFSET;
			int state = (int)GameState::TIMER;

			PackData<int>(packetData, &loc, state);
			PackAuxilaryData(packetData, loc, ~(int)PlayerMask::SERVER, (int)PacketType::STATE);

			// Stop Timer
			for (int i = 0; i < clientCount; i++)
			{
				if (ConnectedUsers[i].active)
				{
					PrintPackInfo("READY CONFIRMATION", i, packetData, loc);
					int sendOK = send(ConnectedUsers[i].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
					if (sendOK == SOCKET_ERROR) {
						std::cout << "Send Error: " << WSAGetLastError() << std::endl;
					}
				}
			}
		}
	}
	else
	{
		allReady = false;

		char packetData[DEFAULT_DATA_SIZE];
		int loc = INITIAL_OFFSET;
		int state = (int)GameState::LOBBY;

		PackData<int>(packetData, &loc, state);
		PackAuxilaryData(packetData, loc, ~(int)PlayerMask::SERVER, (int)PacketType::STATE);

		// Stop Timer
		for (int i = 0; i < clientCount; i++)
		{
			if (ConnectedUsers[i].active)
			{
				PrintPackInfo("UNREADY", i, packetData, loc);
				int sendOK = send(ConnectedUsers[i].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
				if (sendOK == SOCKET_ERROR) {
					std::cout << "Send Error: " << WSAGetLastError() << std::endl;
				}
			}
		}
	}
}

void ServerNetwork::StartGame()
{

	for (int i = 0; i < clientCount; i++)
	{
		if (ConnectedUsers[i].loaded == false)
		{
			allLoaded = false;
			break;
		}
	}

	if (allLoaded)
	{
		char packetData[DEFAULT_DATA_SIZE];
		int loc = INITIAL_OFFSET;

		PackData<int>(packetData, &loc, (int)GameState::GAME);
		PackAuxilaryData(packetData, loc, ~(int)PlayerMask::SERVER, (int)PacketType::STATE);

		for (int i = 0; i < clientCount; i++)
		{
			if (ConnectedUsers[i].active)
			{
				PrintPackInfo("START GAME", i, packetData, loc);
				int sendOK = send(ConnectedUsers[i].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
				if (sendOK == SOCKET_ERROR) {
					std::cout << "Send Error: " << WSAGetLastError() << std::endl;
				}
			}
		}
	}
}

void ServerNetwork::StartLoading(float timer)
{
	std::cout << "Countdown Timer: " << timer << ", DT: " << deltaTime << std::endl;
	if (timer <= 0.0f)
	{
		char packetData[DEFAULT_DATA_SIZE];
		timeOut = 60.f;
		int loc = INITIAL_OFFSET;
		int state = (int)GameState::LOAD;

		PackData<int>(packetData, &loc, state);
		PackAuxilaryData(packetData, loc, ~(int)PlayerMask::SERVER, (int)PacketType::STATE);

		// Start Game
		for (int i = 0; i < clientCount; i++)
		{
			if (ConnectedUsers[i].active)
			{
				int sendOK = send(ConnectedUsers[i].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
				if (sendOK == SOCKET_ERROR) {
					std::cout << "Send Error: " << WSAGetLastError() << std::endl;
				}
			}
		}
		gameLoading = true;
	}
}

void ServerNetwork::SocketListening(SOCKET sock)
{
	//UDP Input Socket
	if (sock == udp) {
	char* buf = new char[MAX_PACKET_SIZE];
	sockaddr_in fromAddr;
	int fromLen = sizeof(fromAddr);

	//recieve packet from socket
	int sError = recvfrom(udp, buf, MAX_PACKET_SIZE, 0, (struct sockaddr*) & fromAddr, &fromLen);

	if (sError == SOCKET_ERROR) {
		std::cout << "UDP Recieve Error: " << WSAGetLastError() << std::endl;
	}
	else {
		packetUDP(buf, fromAddr, fromLen);
		//deseralize socket data into packet
		//Packet packet;
		//packet.deserialize(buf);
		//std::cout << "Received UDP Packet: " << packet.packet_type << std::endl;
		//process connection packet
		//if (packet.packet_type == PacketType::INIT) {
		//	std::cout << "UDP Connection" << std::endl;
		//	//std::vector<std::string> parsedData;
		//	////tokenize
		//	//parsedData = Tokenizer::tokenize(',', packet.data);
		//	//parsedData.insert(parsedData.begin(), std::to_string(packet.sender));
		//	packet_init init;
		//	memcpy(&init, &packet.data, sizeof(packet_init));
		//
		//	acceptNewClient(init.index, fromAddr, fromLen);
		//	break;
		//}
		//else {
		//	//process if not connection
		//	//ProcessUDP(packet);
		//	
		//}
	}

	delete[] buf;
	}
	else if (sock == tcp)
	{
		if (!gameLoading)
		{
			std::cout << "TCP Connection" << std::endl;

			SetReady(false);

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

			char packetData[DEFAULT_DATA_SIZE];
			int loc = INITIAL_OFFSET;
			std::cout << newProfile.index << std::endl;
			PackData<int>(packetData, &loc, newProfile.index);
			PackAuxilaryData(packetData, loc, ~(int)PlayerMask::SERVER, (int)PacketType::INIT);

			PrintPackInfo("TCP INIT", -1, packetData, loc);
			int sendOK = send(newProfile.tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
			if (sendOK == SOCKET_ERROR) {
				std::cout << "Init Send Error: " << WSAGetLastError() << std::endl;
			}
			else
			{
				ConnectedUsers[newProfile.index].active = true;
				std::cout << "TCP Client " << newProfile.index << " has connected." << std::endl;
			}
		}
	}
	//TCP Input Sockets
	else {
		char* buf = new char[MAX_PACKET_SIZE];

		int byteReceived = recv(sock, buf, MAX_PACKET_SIZE, 0);
		//handles disconnection packets
		if (byteReceived <= 0)
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
		else if (byteReceived == SOCKET_ERROR) {
			std::cout << "Recieve Error: " << WSAGetLastError() << std::endl;
		}
		else {
			//process packet data
			//Packet packet;
			//packet.deserialize(buf);
			//ProcessTCP(packet);
			packetTCP(buf);
		}

		delete[] buf;
	}
}

void ServerNetwork::startUpdates()
{

	std::cout << "Server Running..." << std::endl;
	/*
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
				//relay(pack, false);
			}
			if (command == "/testTCP") {
				Packet pack;
				pack.packet_type = MESSAGE;

				std::string test = "sending TCP packet";

				strcpy_s(pack.data, test.c_str() + '\0');
				pack.sender = -1;
				//relay(pack, true);
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
	*/

	while (listening) {

		deltaTime = -std::chrono::duration<float>(previousTime - std::chrono::system_clock::now()).count();
		previousTime = std::chrono::system_clock::now();

		fd_set copy = master;

		timeval nope;
		nope.tv_sec = 0;
		nope.tv_usec = 0;
		int socketCount = select(0, &copy, nullptr, nullptr, &nope);

		//handle all active sockets
		for (int i = 0; i < socketCount; i++)
		{
			// std::cout << "Packet Recieved:";

			SOCKET sock = copy.fd_array[i];

			//Socket Listener
			SocketListening(sock);
		}

		if (allReady && !gameLoading)
		{
			timer -= deltaTime;

			StartLoading(timer);
		}
		else if (!allReady && !gameLoading)
		{
			timer = maxTimer;
		}
		if (gameLoading && !allLoaded)
		{
			if (timeOut > 0.f)
			{
				timeOut -= deltaTime;
			}
			else
			{
				allLoaded = true;
			}
		}
	}
}

void ServerNetwork::PrintPackInfo(const char* extraInfo, int sender, char* data, int dataLen)
{
	int type = 0;
	memcpy(&type, data + (sizeof(int) * 2), sizeof(int));
	std::cout << extraInfo << " -> Sender: " << sender << ", Type: " << type << ", Data: ";
	if (dataLen <= 0 || dataLen > DEFAULT_DATA_SIZE)
	{
		dataLen = 20;
	}
	for (int i = 0; i < dataLen; ++i)
	{
		std::cout << (int)data[i] << "\t";
	}
	std::cout << std::endl;
}

void ServerNetwork::PackString(char* buffer, int* loc, std::string* str)
{
	PackData<int>(buffer, loc, (int)str->size());
	memcpy(buffer + *loc, &(*str)[0], (int)str->size());
	*loc += str->size();
}

void ServerNetwork::UnpackString(char* buffer, int* loc, std::string* str, int *length)
{
	UnpackData<int>(buffer, loc, length);
	str->resize(*length, 0);
	memcpy(&(*str)[0], buffer + *loc, *length);
	*loc += *length;
}

void ServerNetwork::PackAuxilaryData(char* buffer, int length, int receiver, int type, int sender)
{
	int loc = 0;
	PackData<int>(buffer, &loc, length);
	PackData<int>(buffer, &loc, receiver);
	PackData<int>(buffer, &loc, type);
	PackData<int>(buffer, &loc, sender);
}

bool ServerNetwork::ChangeType(PlayerType requestedType)
{
	switch (requestedType)
	{
	case PlayerType::OTHER:
		return true;
		break;
	case PlayerType::FPS:
		if (fpsPlayers < 3)
		{
			++fpsPlayers;
			return true;
		}
		return false;
		break;
	case PlayerType::RTS:
		if (rtsPlayers < 1)
		{
			++rtsPlayers;
			return true;
		}
		return false;
		break;
	}

	return false;
}

void ServerNetwork::packetTCP(char* packet)
{
	int receiver = 0;
	memcpy(&receiver, packet + sizeof(int), sizeof(receiver));

	if (receiver & PlayerMask::SERVER)
	{
		int packetType;
		memcpy(&packetType, packet + (sizeof(int) * 2), sizeof(packetType));
		int sender;
		memcpy(&sender, packet + (sizeof(int) * 3), sizeof(sender));
		switch (packetType)
		{
		case PacketType::INIT:
		{

		}
		break;
		case PacketType::USER:
		{
			int loc = INITIAL_OFFSET;
			int id = 0;
			int strLen = 0;
			std::string name = "";

			UnpackData<int>(packet, &loc, &id);
			UnpackString(packet, &loc, &name, &strLen);

			ConnectedUsers[id].Username = name;
			std::cout << "User: " << ConnectedUsers[id].Username << std::endl;

			char packetData[DEFAULT_DATA_SIZE];
			loc = INITIAL_OFFSET;

			for (int i = 0; i < clientCount; i++)
			{
				if (ConnectedUsers[i].active)
				{
					std::cout << "Active id: " << i << std::endl;
					PackData<int>(packetData, &loc, ConnectedUsers[i].index);
					PackString(packetData, &loc, &ConnectedUsers[i].Username);
				}
			}

			PackAuxilaryData(packetData, loc, ~(int)PlayerMask::SERVER, (int)PacketType::USER);
			PrintPackInfo("User Relay", -1, packetData, loc);
			
			int sendOK = send(ConnectedUsers[id].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
			if (sendOK == SOCKET_ERROR) {
				std::cout << "Send Error: " << WSAGetLastError() << std::endl;
			}
		}
		break;
		case PacketType::TYPE:
		{
			if (ConnectedUsers[sender].ready)
			{
				break;
			}

			int playerType;
			memcpy(&playerType, packet + (sizeof(int) * 4), sizeof(playerType));
			// Check for type and send confirmation - Receive Type, Do check, Send back type or send other type
			int currentType = (int)ConnectedUsers[sender].type;

			std::cout << "Current Type: " << currentType << ", Requested Type: " << playerType
				<< "RTS Players: " << rtsPlayers << ", FPS Players: " << fpsPlayers << std::endl;
			if (currentType == playerType)
			{
				ConnectedUsers[sender].type = (PlayerType)OTHER;
				switch (currentType)
				{
				case PlayerType::FPS:
					--fpsPlayers;
					break;
				case PlayerType::RTS:
					--rtsPlayers;
					break;
				}
			}
			else
			{
				switch (currentType)
				{
				case PlayerType::OTHER:
					if (ChangeType((PlayerType)playerType))
					{
						ConnectedUsers[sender].type = (PlayerType)playerType;
					}
					break;
				case PlayerType::RTS:
					if (ChangeType((PlayerType)playerType))
					{
						--rtsPlayers;
						ConnectedUsers[sender].type = (PlayerType)playerType;
					}
					break;
				case PlayerType::FPS:
					if (ChangeType((PlayerType)playerType))
					{
						--fpsPlayers;
						ConnectedUsers[sender].type = (PlayerType)playerType;
					}
					break;
				}
			}

			char packetData[DEFAULT_DATA_SIZE];
			int loc = INITIAL_OFFSET;

			PackData<int>(packetData, &loc, (int)ConnectedUsers[sender].type);
			PackAuxilaryData(packetData, loc, ~(int)PlayerMask::SERVER, (int)PacketType::TYPE, sender);

			for (int i = 0; i < clientCount; i++)
			{
				if (ConnectedUsers[i].active)
				{
					PrintPackInfo("TYPE CONFIRMATION", i, packetData, loc);
					int sendOK = send(ConnectedUsers[i].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
					if (sendOK == SOCKET_ERROR) {
						std::cout << "Send Error: " << WSAGetLastError() << std::endl;
					}
				}
			}
		}
		break;
		case PacketType::READY:
		{
			bool ready;
			memcpy(&ready, packet + (sizeof(int) * 4), sizeof(ready));
			ConnectedUsers[sender].ready = ready;
			SetReady(ready);
			//if (ready)
			//{
			//	allReady = true;
			//	// Check all other readys
			//	for (int i = 0; i < clientCount; i++)
			//	{
			//		if (!ConnectedUsers[i].ready)
			//		{
			//			allReady = false;
			//			break;
			//		}
			//	}
			//	if (allReady)
			//	{
			//		char packetData[DEFAULT_DATA_SIZE];
			//		int loc = INITIAL_OFFSET;
			//		int state = (int)GameState::TIMER;
			//
			//		PackData(packetData, &loc, state);
			//		PackAuxilaryData(packetData, 20, ~(int)PlayerMask::SERVER, (int)PacketType::STATE);
			//
			//		// Stop Timer
			//		for (int i = 0; i < clientCount; i++)
			//		{
			//			if (ConnectedUsers[i].active)
			//			{
			//				PrintPackInfo("READY CONFIRMATION", i, packetData, 20);
			//				int sendOK = send(ConnectedUsers[i].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
			//				if (sendOK == SOCKET_ERROR) {
			//					std::cout << "Send Error: " << WSAGetLastError() << std::endl;
			//				}
			//			}
			//		}
			//	}
			//}
			//else
			//{
			//	allReady = false;
			//
			//	char packetData[DEFAULT_DATA_SIZE];
			//	int loc = INITIAL_OFFSET;
			//	int state = (int)GameState::LOBBY;
			//
			//	PackData(packetData, &loc, state);
			//	PackAuxilaryData(packetData, 20, ~(int)PlayerMask::SERVER, (int)PacketType::STATE);
			//
			//	// Stop Timer
			//	for (int i = 0; i < clientCount; i++)
			//	{
			//		if (ConnectedUsers[i].active)
			//		{
			//			PrintPackInfo("UNREADY", i, packetData, 20);
			//			int sendOK = send(ConnectedUsers[i].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
			//			if (sendOK == SOCKET_ERROR) {
			//				std::cout << "Send Error: " << WSAGetLastError() << std::endl;
			//			}
			//		}
			//	}
			//}
		}
		break;
		case PacketType::STATE:
		{
			int state;
			memcpy(&state, packet + (sizeof(int) * 4), sizeof(state));
			switch (state)
			{
			case (int)GameState::LOAD:
				if (!allLoaded)
				{
					ConnectedUsers[sender].loaded = true;
					allLoaded = true;
					StartGame();

					break;
				}
			}
		}
		break;
		}
	}

	int length;
	memcpy(&length, packet, 4);
	PrintPackInfo("BYPASS", -1, packet, length);

	for (int i = 0; i < clientCount; ++i)
	{
		if (receiver & (1 << (i + 1)))
		{
			if (ConnectedUsers[i].active)
			{
				//int length;
				//memcpy(&length, packet, 4);
				PrintPackInfo("RELAY TCP", i, packet, length);
				int sendOK = send(ConnectedUsers[i].tcpSocket, packet, DEFAULT_DATA_SIZE, 0);
				if (sendOK == SOCKET_ERROR) {
					std::cout << "Send Error: " << WSAGetLastError() << std::endl;
				}
			}
		}
	}
}

void ServerNetwork::packetUDP(char* packet, sockaddr_in fromAddr, int fromLen)
{
	int receiver = 0;
	memcpy(&receiver, packet + sizeof(int), sizeof(receiver));

	if (receiver & PlayerMask::SERVER)
	{
		int packetType;
		memcpy(&packetType, packet + (sizeof(int) * 2), sizeof(packetType));
		int index;
		memcpy(&index, packet + (sizeof(int) * 3), sizeof(index));
		if (!ConnectedUsers[index].activeUDP)
		{
			acceptNewClient(index, fromAddr, fromLen);
		}

		//if (index == (clientCount - 1) && clientCount != 1)
		//{
		//	for (int i = 0; i < index; ++i)
		//	{
		//		if (ConnectedUsers[i].active)
		//		{
		//			char packetData[DEFAULT_DATA_SIZE];
		//			int loc = INITIAL_OFFSET;
		//
		//			PackData(packetData, &loc, i);
		//			PackData(packetData, &loc, ConnectedUsers[i].Username);
		//			PackAuxilaryData(packetData, loc, (1 << index), (int)PacketType::INIT, i);
		//			int sendOK = send(ConnectedUsers[index].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
		//			if (sendOK == SOCKET_ERROR) {
		//				std::cout << "Send Error: " << WSAGetLastError() << std::endl;
		//			}
		//		}
		//	}
		//}

		switch (packetType)
		{
		case PacketType::ENTITY:
			break;
		}
	}
	for (int i = 0; i < clientCount; ++i)
	{
		if (receiver & (1 << (i + 1)))
		{
			if (ConnectedUsers[i].active)
			{
				int length;
				memcpy(&length, packet, 4);
				PrintPackInfo("RELAY UDP", i, packet, length);
				int sendOK = sendto(udp, packet, DEFAULT_DATA_SIZE, 0, (sockaddr*)&ConnectedUsers[i].udpAddress, ConnectedUsers[i].clientLength);
				if (sendOK == SOCKET_ERROR) {
					std::cout << "Send Error: " << WSAGetLastError() << std::endl;
				}
			}
		}
	}
}

void ServerNetwork::SwapIndex(int current, int target)
{
	UserProfile tmp = ConnectedUsers[target];
	ConnectedUsers[target] = ConnectedUsers[current];
	ConnectedUsers[current] = tmp;
}
//
//void ServerNetwork::sendToAll(Packet pack)
//{
//	for (UserProfile client : ConnectedUsers) {
//		const unsigned int packet_size = sizeof(pack);
//		char packet_data[packet_size];
//
//		pack.serialize(packet_data);
//
//		int sendOK = sendto(udp, packet_data, packet_size, 0, (sockaddr*)&client.udpAddress, client.clientLength);
//		if (sendOK == SOCKET_ERROR) {
//			std::cout << "Send Error: " << WSAGetLastError() << std::endl;
//		}
//	}
//}
//
////the UDP Send
//void ServerNetwork::sendTo(Packet pack, int clientID)
//{
//	const unsigned int packet_size = sizeof(pack);
//	char packet_data[packet_size];
//
//	pack.serialize(packet_data);
//
//	int sendOK = sendto(udp, packet_data, packet_size, 0, (sockaddr*)&ConnectedUsers[clientID - 1].udpAddress, ConnectedUsers[clientID - 1].clientLength);
//	if (sendOK == SOCKET_ERROR) {
//		std::cout << "Send Error: " << WSAGetLastError() << std::endl;
//	}
//
//}
//
////the TCP Send
//void ServerNetwork::sendTo(Packet pack, SOCKET client)
//{
//	char packet_data[sizeof(pack)];
//
//	pack.serialize(packet_data);
//	int sendOK = send(client, packet_data, sizeof(pack), 0);
//	if (sendOK == SOCKET_ERROR) {
//		std::cout << "Send Error: " << WSAGetLastError() << std::endl;
//	}
//}
//
/*
//processes all TCP Packets
void ServerNetwork::ProcessTCP(Packet pack)
{
	//std::vector<std::string> parsedData;

	//packet processing 
	switch (pack.packet_type)
	{
	case PacketType::INIT:
		std::cout << "Data Pipeline Error: INIT Connection not handled correctly";
		break;
		//case PacketType::JOIN:
		//	packet_join join;
		//	memcpy(&join, &pack.data, sizeof(packet_join));
		//	if (!join.type)
		//	{
		//		if (!rtsExists)
		//		{
		//			if (join.playerID != 0)
		//			{
		//				SwapIndex(join.playerID, 0);
		//				join.playerID = 0;
		//			}
		//			rtsExists = true;
		//			sendTo(pack, ConnectedUsers[0].tcpSocket);
		//		}
		//		else
		//		{
		//			std::cout << "RTS already exists: " << join.playerID << std::endl;
		//		}
		//	}
		//	else if (join.type)
		//	{
		//		if (rtsExists)
		//		{
		//			if (join.playerID != 0)
		//			{
		//				sendTo(pack, ConnectedUsers[0].tcpSocket);
		//			}
		//		}
		//	}
		//	break;
		//	//relay the data
	case PacketType::MESSAGE:
		//parsedData = Tokenizer::tokenize(',', pack.data);
		packet_msg msg;
		memcpy(&msg, &pack.data, sizeof(packet_msg));
		std::cout << "TCP Message Recieved from user (" + std::to_string(pack.sender) + "):" << std::string(msg.message) << std::endl;

		relay(pack, true);
		break;

	case PacketType::WEAPON:
	case PacketType::BUILD:
	case PacketType::DEATH:
	case PacketType::STATE:
		std::cout << "TCP Packet Type: " << pack.packet_type << " , ID: " << pack.id << std::endl;
		relay(pack, true);
		break;

		//send the data to RTS player
	case PacketType::DAMAGE:
		packet_damage dmg;
		memcpy(&dmg, &pack.data, sizeof(packet_damage));
		if (!dmg.dir)
		{
			sendTo(pack, ConnectedUsers[0].tcpSocket);
			break;
		}
		else
		{
			//send the data to sepific player
			//parsedData = Tokenizer::tokenize(',', pack.data);
			sendTo(pack, ConnectedUsers[dmg.playerID].tcpSocket);
			break;
		}

		//case PacketType::PLAYER_DATA:
		//	std::cout << "Data Protocol Use Invalid: PLAYER_DATA:TCP";
		//	break;
	case PacketType::ENTITY:
		std::cout << "Data Protocol Use Invalid: ENTITY_DATA:TCP";
		break;
		//case PacketType::TURRET_DATA:
		//	std::cout << "Data Protocol Use Invalid: TURRET_DATA:TCP";
		//	break;
	default:
		std::cout << "Error: TCP Unhandled Packet Type: " << pack.packet_type << std::endl;
		break;
	}
}

//processes all UDP Packets
void ServerNetwork::ProcessUDP(Packet pack)
{
	//std::vector<std::string> parsedData;

	switch (pack.packet_type)
	{
		//relay the data to all clients
	case PacketType::MESSAGE:
		packet_msg msg;
		memcpy(&msg, &pack.data, sizeof(packet_msg));
		std::cout << "UDP Message Recieved from user (" + std::to_string(pack.sender) + "):" << std::string(msg.message) << std::endl;

		relay(pack);

		break;
	case PacketType::ENTITY:
		//case PacketType::TURRET_DATA:
		//	// std::cout << "UDP Packet Type: " << pack.packet_type << " , ID: " << pack.id << std::endl;
		//case PacketType::PLAYER_DATA:
		relay(pack, false);
		break;


	case PacketType::INIT:
		std::cout << "Error: Incomming connection packet through invalid TCP channels";
		break;
	case PacketType::WEAPON:
		std::cout << "Data Protocol Use Invalid: WEAPON_DATA:UDP";
		break;
	case PacketType::DAMAGE:
		std::cout << "Data Protocol Use Invalid: DAMAGE_DEALT:UDP";
		break;
	case PacketType::BUILD:
		std::cout << "Data Protocol Use Invalid: BUILD_ENTITY:UDP";
		break;
	case PacketType::DEATH:
		std::cout << "Data Protocol Use Invalid: KILL_ENTITY:UDP";
		break;
	case PacketType::STATE:
		std::cout << "Data Protocol Use Invalid: GAME_STATE:UDP";
		break;
		//case PacketType::PLAYER_DAMAGE:
		//	std::cout << "Data Protocol Use Invalid: PLAYER_DAMAGE:UDP";
		//	break;
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
*/
//void ServerNetwork::printOut(Packet pack, int clientID)
//{
//	std::vector<std::string> parsedData;
//	parsedData = Tokenizer::tokenize(',', pack.data);
//
//	for (int i = 0; i < parsedData.size(); i++) {
//		std::cout << parsedData[i] << std::endl;
//	}
//}
#include "ServerNetwork.h"

UserMetrics* UserMetrics::instance = 0;

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
	serverTCP.sin_port = htons(55555);
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
	serverUDP.sin_port = htons(60000);
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

void ServerNetwork::acceptTCPConnection()
{
	std::cout << "TCP Connection" << std::endl;

	sockaddr_in tmpAddress;
	int tmpLen = sizeof(tmpAddress);

	// Accept a new connection
	SOCKET client = accept(tcp, (struct sockaddr*) & tmpAddress, &tmpLen);
	std::cout << "Client TCP Attempt: " << client << std::endl;

	bool isExist = false;
	for (int i = 0; i < clientCount; i++)
	{
		if (ConnectedUsers[i].tcpAddress.sin_addr.S_un.S_addr == tmpAddress.sin_addr.S_un.S_addr)
		{
			isExist = true;
			std::cout << "Socket Exists: " << i << std::endl;
		}
	}

	if (!isExist && clientCount <= clientLimit)
	{
		SetReady(false);

		//create new profile
		UserProfile newProfile = UserProfile();
		newProfile.tcpSocket = client;
		newProfile.tcpAddress = tmpAddress;
		newProfile.tcpLength = tmpLen;

		if (clientCount < 9) {
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
	else
	{
		std::cout << "User Already Exists" << std::endl;
		closesocket(client);
	}
}

void ServerNetwork::acceptNewClient(int sender, sockaddr_in address, int length)
{
	//processing must be done here
	if (sender < ConnectedUsers.size()) {
		if (address.sin_addr.S_un.S_addr != ConnectedUsers[sender].udpAddress.sin_addr.S_un.S_addr)
		{
			ConnectedUsers[sender].udpAddress = address;
			ConnectedUsers[sender].udpLength = length;

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
		if (allReady && rtsPlayers == 1)
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

void ServerNetwork::StartLoading(float timer)
{
	std::cout << "Countdown Timer: " << timer << ", DT: " << deltaTime << std::endl;
	if (timer <= 0.0f)
	{
		if (ConnectedUsers[0].type != PlayerType::RTS && rtsPlayers != 0)
		{
			for (int i = 1; i < clientCount; i++)
			{
				if (ConnectedUsers[i].type == PlayerType::RTS)
				{
					SwapIndex(0, i);
				}
			}
			UserMetrics::getInstance()->initialize(ConnectedUsers);
		}
		else
		{
			// Need to add expection handling
			std::cout << "@@@@@ NO RTS Player! @@@@@" << std::endl;
		}

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

void ServerNetwork::EndGame()
{
	char packetData[DEFAULT_DATA_SIZE];
	int loc = INITIAL_OFFSET;

	PackData<int>(packetData, &loc, (int)GameState::ENDGAME);
	PackAuxilaryData(packetData, loc, ~(int)PlayerMask::SERVER, (int)PacketType::STATE);

	for (int i = 0; i < clientCount; i++)
	{
		if (ConnectedUsers[i].active)
		{
			PrintPackInfo("GAME ENDED", i, packetData, loc);
			int sendOK = send(ConnectedUsers[i].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
			if (sendOK == SOCKET_ERROR) {
				std::cout << "Send Error: " << WSAGetLastError() << std::endl;
			}
		}
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
		}

		delete[] buf;
	}
	else if (sock == tcp)
	{
		if (!gameLoading)
		{
			acceptTCPConnection();
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
	memcpy(&type, data + PACKET_TYPE, sizeof(int));
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

void ServerNetwork::UnpackString(char* buffer, int* loc, std::string* str, int* length)
{
	UnpackData<int>(buffer, loc, length);
	str->resize(*length, 0);
	memcpy(&(*str)[0], buffer + *loc, *length);
	*loc += *length;
}

void ServerNetwork::PackAuxilaryData(char* buffer, int length, int receiver, int type, int sender)
{
	int loc = 0;
	PackData<int>(buffer, &loc, OK_STAMP);
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
	int stamp;
	memcpy(&stamp, packet + PACKET_STAMP, sizeof(stamp));
	if (stamp != OK_STAMP)
		return;

	int length;
	memcpy(&length, packet + PACKET_LENGTH, sizeof(length));

	std::cout << "LENGTH: " << length << std::endl;

	if (length >= INITIAL_OFFSET && length < 5000)
	{
		int receiver = 0;
		memcpy(&receiver, packet + PACKET_RECEIVERS, sizeof(receiver));
		//if (true)
		//{
		//	int length = 0;
		//	memcpy(&length, packet, sizeof(length));
		//	PrintPackInfo("DEFAULT GET", -69, packet, length);
		//}

		if (receiver & PlayerMask::SERVER)
		{
			int packetType;
			memcpy(&packetType, packet + PACKET_TYPE, sizeof(packetType));
			int sender;
			memcpy(&sender, packet + PACKET_SENDER, sizeof(sender));
			if (sender >= 0 && sender < ConnectedUsers.size())
			{
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

					if (id < ConnectedUsers.size() && id >= 0)
					{
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
				}
				break;
				case PacketType::TYPE:
				{
					if (ConnectedUsers[sender].ready)
					{
						break;
					}

					int playerType;
					memcpy(&playerType, packet + (sizeof(int) * 5), sizeof(playerType));
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
					memcpy(&ready, packet + (sizeof(int) * 5), sizeof(ready));
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
					memcpy(&state, packet + (sizeof(int) * 5), sizeof(state));
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
					case (int)GameState::ENDGAME:
						if (!gameEnded)
						{
							EndGame();
							UserMetrics::getInstance()->writeToFile();
							gameEnded = true;
						}
						break;
					}
				}
				break;
				case PacketType::DAMAGE:
				{
					Damage tmp;
					tmp.attacker_type = sender;
					tmp.location;
				}
				break;
				case PacketType::BUILD:
				{
					Buildings tmp;
				}
				case PacketType::DEATH:
				{
					Death tmp;
				}
				break;
				default:
					break;
				}
			}
		}

		//int length;
		//memcpy(&length, packet, 4);
		//PrintPackInfo("BYPASS", -1, packet, length);

		for (int i = 0; i < clientCount; ++i)
		{
			if (receiver & (1 << (i + 1)))
			{
				if (ConnectedUsers[i].active)
				{
					//int length;
					//memcpy(&length, packet, 4);
					PrintPackInfo("RELAY TCP", i, packet, length);
					std::cout << "Receiver: " << receiver << ", i:" << i << std::endl;
					int sendOK = send(ConnectedUsers[i].tcpSocket, packet, DEFAULT_DATA_SIZE, 0);
					if (sendOK == SOCKET_ERROR) {
						std::cout << "Send Error: " << WSAGetLastError() << std::endl;
					}
				}
			}
		}
	}
}

void ServerNetwork::packetUDP(char* packet, sockaddr_in fromAddr, int fromLen)
{
	int stamp;
	memcpy(&stamp, packet + PACKET_STAMP, sizeof(stamp));
	if (stamp != OK_STAMP)
		return;

	int length;
	memcpy(&length, packet + PACKET_LENGTH, sizeof(length));

	if (length >= INITIAL_OFFSET && length < 5000)
	{
		int receiver = 0;
		memcpy(&receiver, packet + PACKET_RECEIVERS, sizeof(receiver));

		if (receiver & PlayerMask::SERVER)
		{
			int packetType;
			memcpy(&packetType, packet + PACKET_TYPE, sizeof(packetType));
			int index;
			memcpy(&index, packet + PACKET_SENDER, sizeof(index));
			if (index >= 0 && index < ConnectedUsers.size())
			{
				if (!ConnectedUsers[index].activeUDP)
				{
					acceptNewClient(index, fromAddr, fromLen);
				}
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
			case PacketType::FIRING:
				break;
			}
		}
		for (int i = 0; i < clientCount; ++i)
		{
			//std::cout << "Receiver: " << receiver << " , Active: " << i << "; " << ConnectedUsers[i].active << std::endl;;
			if (receiver & (1 << (i + 1)))
			{
				if (ConnectedUsers[i].active)
				{
					int length;
					memcpy(&length, packet, 4);
					//PrintPackInfo("RELAY UDP", i, packet, length);
					int sendOK = sendto(udp, packet, DEFAULT_DATA_SIZE, 0, (sockaddr*)&ConnectedUsers[i].udpAddress, ConnectedUsers[i].udpLength);
					if (sendOK == SOCKET_ERROR) {
						std::cout << "Send Error: " << WSAGetLastError() << std::endl;
					}
				}
			}
		}
	}
}

void ServerNetwork::SwapIndex(int current, int target)
{
	UserProfile tmp = ConnectedUsers[target];

	char packetData[DEFAULT_DATA_SIZE];
	int loc = INITIAL_OFFSET;
	PackData<int>(packetData, &loc, target);
	PackAuxilaryData(packetData, loc, ~(int)PlayerMask::SERVER, (int)PacketType::INIT);

	PrintPackInfo("TCP INIT", -1, packetData, loc);
	int sendOK = send(ConnectedUsers[current].tcpSocket, packetData, DEFAULT_DATA_SIZE, 0);
	if (sendOK == SOCKET_ERROR) {
		std::cout << "Init Send Error: " << WSAGetLastError() << std::endl;
	}

	char packetData2[DEFAULT_DATA_SIZE];
	int loc2 = INITIAL_OFFSET;
	PackData<int>(packetData2, &loc2, current);
	PackAuxilaryData(packetData2, loc2, ~(int)PlayerMask::SERVER, (int)PacketType::INIT);

	PrintPackInfo("TCP INIT", -1, packetData2, loc2);
	int sendOK2 = send(ConnectedUsers[target].tcpSocket, packetData2, DEFAULT_DATA_SIZE, 0);
	if (sendOK2 == SOCKET_ERROR) {
		std::cout << "Init Send Error: " << WSAGetLastError() << std::endl;
	}

	ConnectedUsers[target] = ConnectedUsers[current];
	ConnectedUsers[current] = tmp;
}

void UserMetrics::initialize(std::vector<UserProfile> connected_users)
{
	if (!init)
	{
		for (int i = 0; i < MAX_SAVE_FILES; i++)
		{
			if (FILE* file = fopen((file_name + std::to_string(i) + file_type).c_str(), "r"))
			{
				fclose(file);
			}
			else
			{
				session_id = i;
				current_file_path = file_name + std::to_string(i) + file_type;
				break;
			}
		}

		for (const auto& user : connected_users)
		{
			PlayerData tmp;
			tmp.player_name = user.Username;
			tmp.player_type = user.type;
			tmp.alive_timer = 0.0f;
			player_list.push_back(std::make_shared<PlayerData>(tmp));
		}

		init = true;
	}
}

void UserMetrics::recordDamage(int id, Damage dmg)
{
	player_list[id]->damages.push_back(dmg);
	player_list[id]->total_damage_dealt += dmg.damage;
}

void UserMetrics::recordDeath(int id, Death death)
{
	player_list[id]->deaths.push_back(death);
}

void UserMetrics::recordKill(int id, int target_type)
{
	player_list[id]->total_kills++;
	player_list[id]->kill_target_types.push_back(target_type);
}

void UserMetrics::recordBuild(int id, Buildings building)
{
	player_list[id]->buildings.push_back(building);
}

void UserMetrics::recordEarning(int id, Transaction trans)
{
	player_list[id]->credit_earned.push_back(trans);
	player_list[id]->total_credits_earned += trans.amount;
}

void UserMetrics::recordSpending(int id, Transaction trans)
{
	player_list[id]->credit_spent.push_back(trans);
	player_list[id]->total_credits_spent += trans.amount;
}

void UserMetrics::writeToFile()
{
	std::ofstream save_file;
	save_file.open(current_file_path, std::fstream::out);

	for (const auto& player : player_list)
	{
		save_file << "Player Name: " << player->player_name << std::endl;
		save_file << "Player Type: " << player->player_type << std::endl;
		save_file << "Total Damage Dealt: " << player->total_damage_dealt << std::endl;
		save_file << "Total Kills: " << player->total_kills << std::endl;
		save_file << "Total Credits Earned: " << player->total_credits_earned << std::endl;
		save_file << "Total Credits Spent: " << player->total_damage_dealt << std::endl;
		if (player->player_type == PlayerType::RTS)
		{
			save_file << "Total Turrets: " << player->total_turrets << std::endl;
			save_file << "Total Barracks: " << player->total_barracks << std::endl;
			save_file << "Total Droids: " << player->total_droids << std::endl;
			for (const auto& building : player->buildings)
			{
				save_file << "Building " <<
					building.type << " built at " <<
					building.build_locations.x << ", " <<
					building.build_locations.y << ", " <<
					building.build_locations.z << std::endl;
			}
		}
		else if (player->player_type == PlayerType::FPS)
		{
			for (const auto& death : player->deaths)
			{
				save_file << "After " <<
					death.alive_timer << "secs, Killed by " <<
					death.killer_type << " at " <<
					death.death_loc.x << ", " <<
					death.death_loc.y << ", " <<
					death.death_loc.z << std::endl;
			}
			for (const auto& damage : player->damages)
			{
				save_file << "Damaged by " <<
					damage.attacker_type << " at " <<
					damage.location.x << ", " <<
					damage.location.y << ", " <<
					damage.location.z << std::endl;
			}
			for (const auto& loc : player->location_tracking)
			{
				save_file << "Loc: " <<
					loc.x << ", " <<
					loc.y << ", " <<
					loc.z << std::endl;
			}
		}
		for (const auto& earn : player->credit_earned)
		{
			save_file << "Earned " << earn.amount << " from " << earn.target << std::endl;
		}
		for (const auto& spent : player->credit_spent)
		{
			save_file << "Spent " << spent.amount << " on " << spent.target << std::endl;
		}
	}

	save_file.close();
}

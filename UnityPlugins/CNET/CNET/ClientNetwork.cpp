#include "ClientNetwork.h"


//constructor wrapper
CNET_H ClientNetwork* CreateClient() {
	return new ClientNetwork();
}
//destructor wrapper
CNET_H void DeleteClient(ClientNetwork* client) {
	delete client;
}
//connection
CNET_H void Connect(char* ip, char* username, ClientNetwork* client)
{
	string _ip = string(ip);
	client->username = string(username);

	client->connectToServer(_ip);
}

CNET_H void StartUpdating(ClientNetwork* client)
{
	client->startUpdates();
}

CNET_H void SendData(int type, char* message, bool useTCP, ClientNetwork* client)
{
	client->sendData((PacketType)type, message, useTCP);
}

CNET_H int GetPlayerNumber(ClientNetwork* client)
{
	return client->index;
}

CNET_H bool GetInGame(ClientNetwork* client)
{
	return client->inGame;
}

CNET_H bool GetRequestActive(ClientNetwork* client)
{
	return client->requestActive;
}

CNET_H int GetRequesterIndex(ClientNetwork* client)
{
	return client->requesterIndex;
}

//request game from player of index 
CNET_H void RequestGame(int index, ClientNetwork* client)
{
	client->sendData(REQUEST_GAME, to_string(index), true);
}

//send response to request if one is active
CNET_H void RespondToRequest(bool acceptance, ClientNetwork* client)
{
	if (!client->inGame) {
		client->requestActive = false;

		//join the game on local
		if (acceptance) {
			client->inGame = true;
		}

		client->sendData(REQUEST_RESPONSE, to_string(client->requesterIndex) + "," + to_string(acceptance), true);
	}
}
//quits the current game
CNET_H void QuitGame(ClientNetwork* client)
{
	if (client->inGame) {
		client->sendData(GAME_QUIT, "", true);
		client->inGame = false;
	}
}

CNET_H void RequestLobbyData(ClientNetwork* client)
{
	client->sendData(LOBBY_DATA, "", true);
}

CNET_H void RequestSessionData(ClientNetwork* client)
{
	client->sendData(SESSION_DATA, "", true);
}


CNET_H void SetupUDPMessage(void(*action)(int sender, char* data))
{
	UDPMessage = action;
}

CNET_H void SetupOnConnect(void(*action)()) 
{
	onConnect = action;
}

CNET_H void SetupOnMessage(void(*action)(char* message))
{
	onMessage = action;
}


ClientNetwork::ClientNetwork()
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

	//2. setup server information
	serverUDP.sin_family = AF_INET;
	serverUDP.sin_port = htons(54222);
	serverlength = sizeof(serverUDP);

	udp = socket(AF_INET, SOCK_DGRAM, 0);

	tcp = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}
	serverTCP.sin_family = AF_INET;
	serverTCP.sin_port = htons(54223);
}

ClientNetwork::~ClientNetwork()
{
	listening = false;
	closesocket(tcp);
	closesocket(udp);
	WSACleanup();
}

int ClientNetwork::connectToServer()
{
	return connectToServer(addressDefault);
}

int ClientNetwork::connectToServer(string ip)
{
	ipActual = ip;

	inet_pton(AF_INET, ip.c_str(), &serverTCP.sin_addr);		//connecting to the tcp server

	int connResult = connect(tcp, (sockaddr*)&serverTCP, sizeof(serverTCP));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(tcp);
		WSACleanup();
		return SOCKET_ERROR;
	}

	//ping and determine client index
	return 0;
}

int ClientNetwork::sendData(int packetType, string message, bool useTCP)
{
	//create packet
	Packet packet;
	strcpy_s(packet.data, message.c_str() + '\0');
	packet.packet_type = packetType;
	packet.sender = index;

	//set size
	const unsigned int packet_size = sizeof(packet);
	char packet_data[packet_size];

	//seralize
	packet.serialize(packet_data);

	int sendOK = 0;

	//udp send
	if (!useTCP) {
		sendOK = sendto(udp, packet_data, packet_size, 0, (sockaddr*)&serverUDP, sizeof(serverUDP));
		if (sendOK == SOCKET_ERROR) {
			cout << "Send Error: " << WSAGetLastError() << endl;
			return -1;
		}
	}
	//tcp send
	else {
		int sendResult = send(tcp, packet_data, packet_size, 0);
		if (sendResult == SOCKET_ERROR)
		{
			cout << "Send Error: " << WSAGetLastError() << endl;
			return -1;
		}
	}
	return sendOK;
}

void ClientNetwork::startUpdates()
{
	//multithread
	thread udpUpdate = thread([&]() {
		char* buf = new char[MAX_PACKET_SIZE];

		while (true) {
			//recieve messages
			int length = recvfrom(udp, buf, MAX_PACKET_SIZE, 0, (sockaddr*)&serverUDP, &serverlength);
			if (length != SOCKET_ERROR) {
				Packet packet;
				packet.deserialize(buf);
				ProcessUDP(packet);
			}
		}
		});
	udpUpdate.detach();

	thread tcpUpdate = thread([&]() {
		char* buf = new char[MAX_PACKET_SIZE];

		while (true) {
			//recieve packets
			int length = recv(tcp, buf, MAX_PACKET_SIZE, 0);
			if (length != SOCKET_ERROR) {
				Packet packet;
				packet.deserialize(buf);
				ProcessTCP(packet);
			}
		}

		});
	tcpUpdate.detach();
}
std::vector<std::string> ClientNetwork::tokenize(char token, std::string text)
{
	std::vector<std::string> temp;
	int lastTokenLocation = 0;

	for (int i = 0; i < text.size(); i++) {
		if (text[i] == token) {
			temp.push_back(text.substr(lastTokenLocation, i - lastTokenLocation));
			lastTokenLocation = i + 1;

		}
	}
	temp.push_back(text.substr(lastTokenLocation, text.size()));

	return temp;
}

void ClientNetwork::ProcessTCP(Packet pack)
{
	std::vector<std::string> parsedData;
	parsedData = tokenize(',', pack.data);

	switch (pack.packet_type) {
	case PacketType::INIT_CONNECTION:
		//filter by sender
		if (pack.sender == -1) {
			index = std::stof(parsedData[0]);

			//connect to udp
			inet_pton(AF_INET, ipActual.c_str(), &serverUDP.sin_addr);
			sendData(INIT_CONNECTION, to_string(index) + "," + username, false);
		}
		break;
	case PacketType::MESSAGE:

		onMessage(pack.data);
		break;

	case PacketType::REQUEST_GAME:
		cout << "Recieved Game Request from user: " + parsedData[0] + " Please accept or deny";
		requestActive = true;
		requesterIndex = pack.sender;

		break;
	case PacketType::REQUEST_RESPONSE:
		if (parsedData[0] == "1") {
			cout << "Request for game Accepted by user: " + parsedData[1];
			inGame = true;
		}
		else {
			cout << "Request for game Denied by user: " + parsedData[1];
			inGame = false;
		}

		break;

	default:
		cout << "Error Unhandled Type";

		break;
	}


}

void ClientNetwork::ProcessUDP(Packet pack)
{
	std::vector<std::string> parsedData;
	parsedData = tokenize(',', pack.data);


	switch (pack.packet_type) {
	case PacketType::INIT_CONNECTION:
		index = std::stof(parsedData[0]);
		//call action
		onConnect();
		break;
	case PacketType::MESSAGE:
		//send UDP message
		UDPMessage(pack.sender, pack.data);
		break;


	default:
		cout << "Error Unhandled Type";

		break;
	}

}

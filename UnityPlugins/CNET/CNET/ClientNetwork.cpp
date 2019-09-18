#include "ClientNetwork.h"

//test function
CNET_H int Add(int a, int b) {
	return a + b;
}
//need wrappers for everything

CNET_H void RecieveString(const char* str) {
	string y = string(str);
	y += "Recieved ";
}

CNET_H void SendString(char* str, int length)
{
	string x = "hello";
	strcpy_s(str, length, x.c_str());
}


/*
Client Side Function Wrappers Start Now
############################################################
*/


//constructor wrapper
CNET_H ClientNetwork* CreateClient() {
	return new ClientNetwork();
}
//destructor wrapper
CNET_H void DeleteClient(ClientNetwork* client) {
	delete client;
}
//connection
CNET_H int Connect(char* ip, ClientNetwork* client)
{
	string _ip = string(ip);
	client->connect(_ip);
}

CNET_H void StartUpdating(ClientNetwork* client)
{
	client->startUpdates();
}

CNET_H void SendMsg(char* message, ClientNetwork* client)
{
	client->sendMessage(message);
}

CNET_H void SetupPacketReception(void(*action)(int type, int sender, char* data))
{
	recievePacket = action;
}



/*
Client Network Functions Start Now
############################################################
*/

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
	server.sin_family = AF_INET;
	server.sin_port = htons(54000);
	serverlength = sizeof(server);

	//3. setup socket
	client = socket(AF_INET, SOCK_DGRAM, 0);

	//initalization
	connectionsIn = vector<std::vector<std::string>>();
	messagesIn = vector<std::vector<std::string>>();

}

ClientNetwork::~ClientNetwork()
{
	closesocket(client);
}

int ClientNetwork::connect()
{
	return connect(addressDefault);
}

int ClientNetwork::connect(string ip)
{
	inet_pton(AF_INET, ip.c_str(), &server.sin_addr);		//connecting to the server
	//init message
	sendData(INIT_CONNECTION, "0");
	//ping and determine client index
	return 0;
}

int ClientNetwork::sendData(int packetType, string message)
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

	//send to server
	int sendOK = sendto(client, packet_data, packet_size, 0, (sockaddr*)& server, sizeof(server));
	if (sendOK == SOCKET_ERROR) {
		cout << "Send Error: " << WSAGetLastError() << endl;
		return -1;
	}
	return sendOK;
}

void ClientNetwork::startUpdates()
{
	/*
	std::thread thread_name_here(&ClientNetwork::sendMessage, this, "1278.0.1");
	thread_name_here.detach();
	*/

	//multithread
	thread listen = thread([&]() {
		char* buf = new char[MAX_PACKET_SIZE];

		while (true) {

			
			//recieve messages
			int length = recvfrom(client, buf, MAX_PACKET_SIZE, 0, (sockaddr*)& server, &serverlength);
			if (length != SOCKET_ERROR) {
				Packet packet;
				std::vector<std::string> parsedData;

				int i = 0;
				while (i < (unsigned int)length) {
					packet.deserialize(&(buf[i]));
					i += sizeof(Packet);

					//process connections in dll
					if (packet.packet_type == PacketType::INIT_CONNECTION) {
						parsedData = tokenize(',', packet.data);
						parsedData.insert(parsedData.begin(), to_string(packet.sender));
						connectionsIn.push_back(parsedData);
					}
					//process everything else in unity
					else {
						recievePacket(packet.packet_type, packet.sender, packet.data);
					}
				}
			}
			//process connections
			for (std::vector<std::string> parsedData : connectionsIn) {
				int sender = std::stoi(parsedData[0]);

				//filter by sender
				if (sender == 0) {
					index = std::stof(parsedData[1]);
				}
				else {
					//do nothing
				}
			}
			connectionsIn.clear();

			//messages are processed in #
			
			/*
			//process messages
			for (std::vector<std::string> parsedData : messagesIn) {
				int sender = std::stoi(parsedData[0]);

				//filter by sender
				if (sender == 0) {
					string message = "";
					for (int counter = 1; counter < parsedData.size(); counter++){
						message = message + parsedData[counter];
					}
					cout << "Message Recieved from Server :" << message << endl;
				}
				else {
					string message = "";
					for (int counter = 1; counter < parsedData.size(); counter++) {
						message = message + parsedData[counter];
					}
					cout << "Message Recieved from Client"<< parsedData[0] << " :" << message << endl;
				}
			}
			messagesIn.clear();
			*/
		}
		});
	listen.detach();
}

int ClientNetwork::sendMessage(string message)
{
	message = message + ",";

	return sendData(MESSAGE, message);
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
	return temp;
}
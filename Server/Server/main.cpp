#include <iostream>
#include <iostream>
#include <WS2tcpip.h>
#include <map>
#include <vector> 

using namespace std;

#pragma comment (lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "6883" 

using namespace std;

bool running = true;

void main() {


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
	SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);		//one of these for each client	//doesnt know when client disconnects	//make sure to send pings
	//bind socket to ip address and port
	sockaddr_in serverHint;							//info about the client
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(54000); //convert from little to big endian

	if (bind(in, (sockaddr*)& serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		cout << "Can't bind socket! " << WSAGetLastError() << endl;					//bind client info to client
	}


	char buf[99999];
	//ZeroMemory(buf, 1024);

	//runtime loop
	while (running) {
		//wait for message
		int clientLength = sizeof(serverHint);
		if (recvfrom(in, buf, 1024, 0, (sockaddr*)&serverHint, &clientLength) == SOCKET_ERROR) {
			cout << "Error reciving from client" << WSAGetLastError() << endl;
			continue;
		}
		
		if (buf[0] == '1') {
			string s = "Recieved";

			int sendOK = sendto(in, s.c_str(), s.size() + 1, 0, (sockaddr*)& serverHint, clientLength);
			if (sendOK == SOCKET_ERROR) {
				cout << "Send Error: " << WSAGetLastError() << endl;
			}

		}

		//display message and client info
		char x = buf[0];
		string clientIP;
		clientIP = x;

		cout << "Message Recieved from " << clientIP << " : " << buf << endl;
	}

	

	//close
	closesocket(in);
	WSACleanup();

}
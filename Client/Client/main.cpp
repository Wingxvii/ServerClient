#include <iostream>
#include <ws2tcpip.h>
#include <string>
#include "ClientNetwork.h"
#pragma comment (lib, "ws2_32.lib")
#define MAX_PACKET_SIZE 10000

using namespace std;

bool running = true;

int main() {
	
	//init network
	ClientNetwork net = ClientNetwork();

	//connect to default ip address
	net.connect();
	net.startUpdates();


	while (true) {
		string s;
		cin >> s;
		net.sendMessage(MESSAGE, s);

	}


	//shut down 1
	WSACleanup();
}
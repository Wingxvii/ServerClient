#include <iostream>
#include <string>
#include "ClientNetwork.h"

using namespace std;

bool running = true;

int main() {
	
	//init network
	ClientNetwork net = ClientNetwork();

	//connect to default ip address
	net.connectToServer();

	net.startUpdates();


	while (true) {
		string s;
		cin >> s;

		net.sendMessage(s, false);
	}


	//shut down 1
	WSACleanup();
}
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
		string command;
		cin >> command;

		if (command == "/quit") {
			net.listening = false;
		}
		if (command == "/test") {

			net.sendData(MESSAGE, "hello", false);
		}
	}


	//shut down 1
	WSACleanup();
}
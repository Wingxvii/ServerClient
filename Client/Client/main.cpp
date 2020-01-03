#include <iostream>
#include <string>
#include "ClientNetwork.h"

using namespace std;

bool running = true;

int main() {
	
	//init network
	ClientNetwork net = ClientNetwork();

	//connect to default ip address
	net.connect("127.0.0.1");

	net.startUpdates();


	while (true) {
		string s;
		cin >> s;

		net.sendMessage(s);
	}


	//shut down 1
	WSACleanup();
}
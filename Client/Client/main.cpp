#include <iostream>
#include <string>
#include "ClientNetwork.h"

using namespace std;

bool running = true;

int main() {
	
	//init network
	ClientNetwork net = ClientNetwork();

	string ip;
	cout << "Address:";
	cin >> ip;

	//connect to default ip address
	if (ip == "0") {
		net.connect();
	}
	else {
		net.connect(ip);
	}

	net.startUpdates();


	while (true) {
		string s;
		cin >> s;

		net.sendMessage(s);
	}


	//shut down 1
	WSACleanup();
}
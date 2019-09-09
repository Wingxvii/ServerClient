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

	string index;
	cout << "Input Index: ";
	cin >> index;
	net.setClientIndex(index);

	//connect to default ip address
	net.connect();

	while (true) {
		//write out to that socket
		string s;
		cout << "Send Message: ";
		cin >> s;

		net.sendMessage(s);

		net.startListening();


		while (!net.inQueue.empty()) {
			char x = net.inQueue.back()[0];
			string message = net.inQueue.back();

			string clientIP;
			clientIP = x;

			cout << "Message Recieved from " << clientIP << " : " << message << endl;
			net.inQueue.pop();
		}
	}


	//shut down 1
	WSACleanup();
}
#include <iostream>
#include <iostream>
#include <WS2tcpip.h>
#include <map>
#include <vector> 
#include "ServerNetwork.h"

using namespace std;

#pragma comment (lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "6883" 

using namespace std;

bool running = true;

void main() {

	ServerNetwork net = ServerNetwork();

	net.startListening();
	
	//runtime loop
	while (running) {
		//wait for message

		while (!net.inQueue.empty()) {
			char x = net.inQueue.back()[0];
			string message = net.inQueue.back();
			/*
			if (x == '1') {
				string s = "Recieved";
				net.sendTo(s, 0, 0);
			}
			*/
			cout << "Message Recieved from " << x << " : " << message << endl;

			net.inQueue.pop();
		}

	}

	//close
	WSACleanup();

}
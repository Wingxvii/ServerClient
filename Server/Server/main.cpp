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
	
	thread print = thread([&]() {
		while (true) {
			while (!net.inQueue.empty()) {
				char x = net.inQueue.back()[0];
				string message = net.inQueue.back();
				string s = "Recieved";
				net.sendTo(s, 0);
				cout << "Message Recieved from " << x << " : " << message << endl;

				net.inQueue.pop();
			}
		}
		});
	print.detach();



	//runtime loop
	while (running) {
		//wait for message
	}

	//close
	WSACleanup();

}
#include <iostream>
#include <iostream>
#include <WS2tcpip.h>
#include <map>
#include <vector> 
#include "ServerNetwork.h"

using namespace std;

bool running = true;

int main() {

	ServerNetwork net = ServerNetwork();

	net.startUpdates();
	
	//runtime loop
	while (running) {
		//wait for message
		//send command
	}

	//close
	WSACleanup();

}


#include <iostream>
#include <map>
#include <vector> 
#include "ServerNetwork.h"


bool running = true;

int main() {

	ServerNetwork net = ServerNetwork();
	net.startUpdates();

}


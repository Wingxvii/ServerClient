#include <iostream>
#include <map>
#include <vector> 
#include "ServerNetwork.h"


bool running = true;

int main() {
	int users = 9;
	std::cout << "Enter the Max Amount of Users: ";
	std::cin >> users;

	ServerNetwork net = ServerNetwork();
	net.setMaxUsers(users);
	net.startUpdates();

}


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
		std::getline(std::cin, command);

		if (command == "/quit") {
			net.listening = false;
		}
		else if (command == "/test") {

			net.sendData(MESSAGE, "hello", false);
		}
		else if (command.substr(0, 5) == "/send") {
			if (command.size() >= 8) {
				if (command.substr(0, 9) == "/send udp") {
					string message = command.erase(0, 9);
					net.sendData(PacketType::MESSAGE, message);

				}
				else if (command.substr(0, 9) == "/send tcp") {
					string message = command.erase(0, 9);
					net.sendData(PacketType::MESSAGE, message, true);
				}
				else {
					string message = command.erase(0, 5);
					net.sendData(PacketType::MESSAGE, message);
				}
			}
			else {
				string message = command.erase(0, 5);
				net.sendData(PacketType::MESSAGE, message);
			}
		}

	}


	//shut down 1
	WSACleanup();
}
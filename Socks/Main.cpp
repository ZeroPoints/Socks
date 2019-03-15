#include <iostream>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <thread>
#include <memory>

#include <cereal/archives/binary.hpp>
#include <sstream>
#include <deque>

#include "ConnectionManager.h"
#include "Payload.h"





/*
Client example
*/
void ClientMainThread(char *ipaddress);
/*
Server example
*/
void ServerMainThread();






int main()
{
	int mode = -1;
	printf("Server - 1\n");
	printf("Client - 2\n");
	printf("Enter a mode:\n");
	std::cin >> mode;

	if (mode == 2)
	{
		char ipaddress[32];
		std::cout << "Enter Server IP:\n";
		std::cin >> ipaddress;
		ClientMainThread(ipaddress);
	}
	else if (mode == 1)
	{
		ServerMainThread();
	}
	printf("System Shut Down Done.\n");
	system("pause");
}




/*
Server example
*/
void ServerMainThread()
{
	std::cout << "Starting Connection Server\n";
	auto connectionManager = new ConnectionManager();
	std::thread serverConnectionThread;
	serverConnectionThread = std::thread(&ConnectionManager::StartServerListener, connectionManager);
	std::cout << "Connection Thread Initiated\n";


	//Stick it in an infinite loop...
	bool done = false;
	while (!done)
	{

		int input = 0;
		std::cout << "\n";
		std::cout << "\n";
		std::cout << "Pick an option\n";
		std::cout << "1: 'extra' type payload \n";
		std::cout << "2: 'status' type payload \n";
		std::cout << "3: 'Info' type payload \n";
		std::cout << "4: Exit \n";
		std::cout << "Choice: ";
		std::cin >> input;

		Payload pl;
		switch (input)
		{
		case 1:
			pl.data = "Thanks for the aids mate";
			pl.type = Payload::PayloadType::Extra;
			connectionManager->PayloadToSendAll(pl);
			break;
		case 2:
			pl.data = "Thanks blah face";
			pl.type = Payload::PayloadType::Status;
			connectionManager->PayloadToSendAll(pl);
			break;
		case 3:
			pl.data = "1ABCDEFGHIJKLMNOPQRSTUVWXYZ2ABCDEFGHIJKLMNOPQRSTUVWXYZ3ABCDEFGHIJKLMNOPQRSTUVWXYZ4ABCDEFGHIJKLMNOPQRSTUVWXYZ5ABCDEFGHIJKLMNOPQRSTUVWXYZ6ABCDEFGHIJKLMNOPQRSTUVWXYZ7ABCDEFGHIJKLMNOPQRSTUVWXYZ8ABCDEFGHIJKLMNOPQRSTUVWXYZ9ABCDEFGHIJKLMNOPQRSTUVWXYZ1ABCDEFGHIJKLMNOPQRSTUVWXYZ2ABCDEFGHIJKLMNOPQRSTUVWXYZ3ABCDEFGHIJKLMNOPQRSTUVWXYZ4ABCDEFGHIJKLMNOPQRSTUVWXYZ5ABCDEFGHIJKLMNOPQRSTUVWXYZ6ABCDEFGHIJKLMNOPQRSTUVWXYZ7ABCDEFGHIJKLMNOPQRSTUVWXYZ8ABCDEFGHIJKLMNOPQRSTUVWXYZ9ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			pl.type = Payload::PayloadType::Info;
			connectionManager->PayloadToSendAll(pl);
			break;
		case 4:
			done = true;
			break;
		default:
			break;
		}
	}
}


/*
Client example
*/
void ClientMainThread(char *ipaddress)
{
	std::cout << "Starting Connection Client\n";
	auto connectionManager = new ConnectionManager();
	std::thread clientConnectionThread;
	clientConnectionThread = std::thread(&ConnectionManager::StartClientConnection, connectionManager, ipaddress, 5001);
	std::cout << "Connection Thread Initiated\n";

	//Stick it in an infinite loop...
	bool done = false;
	while (!done)
	{
		int input = 0;
		std::cout << "\n";
		std::cout << "\n";
		std::cout << "Pick an option\n";
		std::cout << "1: 'extra' type payload \n";
		std::cout << "2: 'status' type payload \n";
		std::cout << "3: 'Info' type payload and a long payload \n";
		std::cout << "4: Exit \n";
		std::cout << "Choice: ";
		std::cin >> input;

		Payload pl;
		switch (input)
		{
		case 1:
			pl.data = "Thanks for the aids mate";
			pl.type = Payload::PayloadType::Extra;
			connectionManager->PayloadToSendAll(pl);
			break;
		case 2:
			pl.data = "Thanks blah face";
			pl.type = Payload::PayloadType::Status;
			connectionManager->PayloadToSendAll(pl);
			break;
		case 3:
			//Send Data To All
			pl.data = "1ABCDEFGHIJKLMNOPQRSTUVWXYZ2ABCDEFGHIJKLMNOPQRSTUVWXYZ3ABCDEFGHIJKLMNOPQRSTUVWXYZ4ABCDEFGHIJKLMNOPQRSTUVWXYZ5ABCDEFGHIJKLMNOPQRSTUVWXYZ6ABCDEFGHIJKLMNOPQRSTUVWXYZ7ABCDEFGHIJKLMNOPQRSTUVWXYZ8ABCDEFGHIJKLMNOPQRSTUVWXYZ9ABCDEFGHIJKLMNOPQRSTUVWXYZ1ABCDEFGHIJKLMNOPQRSTUVWXYZ2ABCDEFGHIJKLMNOPQRSTUVWXYZ3ABCDEFGHIJKLMNOPQRSTUVWXYZ4ABCDEFGHIJKLMNOPQRSTUVWXYZ5ABCDEFGHIJKLMNOPQRSTUVWXYZ6ABCDEFGHIJKLMNOPQRSTUVWXYZ7ABCDEFGHIJKLMNOPQRSTUVWXYZ8ABCDEFGHIJKLMNOPQRSTUVWXYZ9ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			pl.type = Payload::PayloadType::Info;
			connectionManager->PayloadToSendAll(pl);
			break;
		case 4:
			done = true;
			break;
		default:
			break;
		}
	}
	//Need to tell client connection thread to end and clean up

}





#include <iostream>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <thread>

#include <cereal/archives/binary.hpp>
#include <sstream>
#include <deque>

#include "ConnectionManager.h"
#include "Payload.h"

//maybe look at https://www.youtube.com/channel/UC5Lxe7GAsk_f8qMBsNmlOJg JPRES. saw some helpful learning stuff in there voice is annoying though.


void ClientMainThread(char *ipaddress);
void ServerMainThread();


ConnectionManager* connectionManager;

int main()
{
	connectionManager = new ConnectionManager();
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





void ServerMainThread()
{
	std::thread serverConnectionThread;
	serverConnectionThread = std::thread(&ConnectionManager::StartServerListener, connectionManager);

	while (true)
	{
		printf("Main Thread Sleeping\n");
		Sleep(5000);
	}
}



void ClientMainThread(char *ipaddress)
{
	printf("Starting Connection Client\n");
	std::thread clientConnectionThread;
	std::thread threadTest2;

	clientConnectionThread = std::thread(&ConnectionManager::StartClientConnection, connectionManager, ipaddress, 5001);

	//threadTest = std::thread(ThreadConnectionSender, std::ref(connectionManager));

	while (true)
	{
		int test = 0;
		std::cout << "\nType 1 or 2 and push enter: ";
		std::cin >> test;
		Payload pl;
		switch (test)
		{
		case 1:
			pl.data = "Thanks for the aids mate";
			pl.type = Payload::PayloadType::Extra;
			connectionManager->PayloadToSend(pl);
			break;
		case 2:
			pl.data = "Thanks blah face";
			pl.type = Payload::PayloadType::Status;
			connectionManager->PayloadToSend(pl);
			break;
		case 3:
			//Send Data To All
			pl.data = "1ABCDEFGHIJKLMNOPQRSTUVWXYZ2ABCDEFGHIJKLMNOPQRSTUVWXYZ3ABCDEFGHIJKLMNOPQRSTUVWXYZ4ABCDEFGHIJKLMNOPQRSTUVWXYZ5ABCDEFGHIJKLMNOPQRSTUVWXYZ6ABCDEFGHIJKLMNOPQRSTUVWXYZ7ABCDEFGHIJKLMNOPQRSTUVWXYZ8ABCDEFGHIJKLMNOPQRSTUVWXYZ9ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			pl.type = Payload::PayloadType::Info;
			connectionManager->PayloadToSend(pl);
			break;
		default:
			break;
		}
	}
}





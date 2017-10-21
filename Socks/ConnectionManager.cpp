
#include "ConnectionManager.h"





ConnectionManager::ConnectionManager()
{
	WSADATA wsaData;
	// Minimum winsock version required
	const int iReqWinsockVer = 2;
	if (WSAStartup(MAKEWORD(iReqWinsockVer, 0), &wsaData) == 0)
	{
		//Check if major version is at least iReqWinsockVer
		if (LOBYTE(wsaData.wVersion) >= iReqWinsockVer)
		{
			printf("All good in the hood.\n");
		}
		else
		{
			printf("WinSock Version Failed.");
		}
	}
	else
	{
		printf("WSA Startup Failed.");
	}
}


ConnectionManager::~ConnectionManager()
{
	//close all connections
	//Cleanup winsock
	if (WSACleanup() != 0)
	{
		printf("WSA Clean Failed.");
		// cleanup failed
	}
}





void ConnectionManager::StartServerListener()
{

	int flagOn = 1;
	u_long nonBlocking = 1;
	int result = -1;
	//struct timeval timeout;
	//timeout.tv_sec = 5;                         
	//timeout.tv_usec = 10000;

	//maybe dont use tcp?
	mainSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (mainSocket_ == INVALID_SOCKET)
	{
		printf("Socket Init Failed.\n");
		return;
	}

	sockAddr_.sin_family = AF_INET;
	sockAddr_.sin_port = htons(5001);
	//will this mean i cant use external ip or ipv6?
	sockAddr_.sin_addr.s_addr = htonl(INADDR_ANY);

	//reuse socket descriptor
	result = setsockopt(mainSocket_, SOL_SOCKET, SO_REUSEADDR, (char *)&flagOn, sizeof(flagOn));
	if (result < 0)
	{
		printf("Socket setsockopt Failed.\n");
		return;
	}

	//Non Blocking code
	result = ioctlsocket(mainSocket_, FIONBIO, &nonBlocking);
	if (result < 0)
	{
		printf("Socket ioctl Failed.\n");
		return;
	}

	//generic bind statement
	result = bind(mainSocket_, (SOCKADDR *)(&sockAddr_), sizeof(sockAddr_));
	if (result == SOCKET_ERROR)
	{
		printf("Socket bind Failed.\n");
		return;
	}

	//does the value show how many concurrent descriptors i can run?
	result = listen(mainSocket_, 16);
	if (result == SOCKET_ERROR)
	{
		printf("Socket listen Failed.\n");
		return;
	}

	SetMasterDescriptor();

	while (true)
	{
		try
		{
			memcpy(&readfds_, &masterfds_, sizeof(masterfds_));
			memcpy(&writefds_, &masterfds_, sizeof(masterfds_));
			/*FD_ZERO(&writefds_);
			for (auto it = connectionList_.begin(); it != connectionList_.end(); it++)
			{
				if (it->second.DataToTransmit.size() > 0)
				{
					FD_SET(it->first, &writefds_);
				}
			}*/
			result = select(maxFileDescriptors_ + 1, &readfds_, &writefds_, NULL, NULL);
			if (result < 0)
			{
				char errBuff[100];
				strerror_s(errBuff, errno);
				printf("Socket select Failed: %d - %s.\n", result, errBuff);
				//return;
			}

			//Loop for each socket in servers list and send them this message
			for (int fd = 0; fd <= maxFileDescriptors_; fd++)
			{
				if (FD_ISSET(fd, &readfds_))
				{
					if (fd == mainSocket_)
					{
						auto newClientSocket = AcceptConnection();
						FD_SET(newClientSocket, &masterfds_);
						if (newClientSocket > maxFileDescriptors_)
						{
							maxFileDescriptors_ = newClientSocket;
						}
					}
					else
					{
						//For this connection if I already have the size then go get the data
						if (connectionList_[fd].ExpectedSize == 0)
						{
							result = ReceiveExpectedSize(fd);
							if (result == -1)
							{
								CleanConnectionAndDesriptor(fd);
							}
						}
						else
						{
							//Dont try do this straight away
							//Can return -1 if ran to quickly.
							result = ReceiveExpectedData(fd);
							if (result == -1)
							{
								int test = 0;
								CleanConnectionAndDesriptor(fd);
							}
						}
					}
				}

				//Will this keep firing hopefully?
				if (FD_ISSET(fd, &writefds_))
				{
					if (connectionList_[fd].DataToTransmit.size() > 0)
					{
						SendData(fd);
					}
				}
			}
		}
		catch (std::exception const& e)
		{
			printf("Bug StartServerListener: %s\n", e.what());
			Sleep(5000);
		}
	}
}








//Should maybe clean up the connectionList_
//For now i wont just so i can see debug of who has connected in whole runtime.
//Also if fd reuse will override the connection spot anyway
int ConnectionManager::CleanConnectionAndDesriptor(int fd)
{
	closesocket(fd);
	FD_CLR(fd, &masterfds_);
	if (fd == maxFileDescriptors_)
	{
		while (FD_ISSET(maxFileDescriptors_, &masterfds_) == FALSE)
		{
			maxFileDescriptors_--;
		}
	}
	return maxFileDescriptors_;
}














void ConnectionManager::StartClientConnection(char *ipAddress, int port)
{
	u_long nonBlocking = 1;
	int result = -1;
	bool connected = false;
	//struct timeval timeout;
	//timeout.tv_sec = 5;
	//timeout.tv_usec = 10000;

	mainSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mainSocket_ == INVALID_SOCKET)
	{
		printf("Socket Init Failed.");
	}

	Connection newClient = CreateNewConnection(ipAddress, port);

	while (!connected)
	{
		try
		{
			auto connectResult = connect(mainSocket_, (SOCKADDR *)&newClient.SockInfo, sizeof(newClient.SockInfo));
			if (connectResult == SOCKET_ERROR)
			{
				printf("Attempt to connect failed. - %ld \n", WSAGetLastError());
				//clean up current socket
				connected = false;
				Sleep(5000);
				continue;
			}
			connected = true;
			connectionList_[mainSocket_] = newClient;
			printf("New Connection: %d - %s - %d\n", mainSocket_, newClient.IPAddress, newClient.Port);

			//Non Blocking code
			result = ioctlsocket(mainSocket_, FIONBIO, &nonBlocking);
			if (result < 0)
			{
				printf("Socket ioctl Failed.\n");
				return;
			}

			SetMasterDescriptor();

			//Keep sending/receiving data
			while (true)
			{
				try
				{
					memcpy(&readfds_, &masterfds_, sizeof(masterfds_));
					memcpy(&writefds_, &masterfds_, sizeof(masterfds_));
					/*FD_ZERO(&writefds_);
					for (auto it = connectionList_.begin(); it != connectionList_.end(); it++)
					{
						if (it->second.DataToTransmit.size() > 0)
						{
							FD_SET(it->first, &writefds_);
						}
					}*/
					result = select(maxFileDescriptors_ + 1, &readfds_, &writefds_, NULL, NULL);
					if (result < 0)
					{
						char errBuff[100];
						strerror_s(errBuff, errno);
						printf("Socket select Failed: %d - %s.\n", result, errBuff);
						//return;
					}

					//Loop for each socket in servers list and send them this message
					for (int fd = 0; fd <= maxFileDescriptors_; fd++)
					{
						if (FD_ISSET(fd, &readfds_))
						{
							if (connectionList_[mainSocket_].ExpectedSize == 0)
							{
								result = ReceiveExpectedSize(mainSocket_);
								if (result == -1)
								{
									CleanConnectionAndDesriptor(mainSocket_);
								}
							}
							else
							{
								//Dont try do this straight away
								//Can return -1 if ran to quickly.
								result = ReceiveExpectedData(mainSocket_);
								if (result == -1)
								{
									int test = 0;
									CleanConnectionAndDesriptor(mainSocket_);
								}
							}
						}

						//Will this keep firing hopefully?
						if (FD_ISSET(fd, &writefds_))
						{
							if (connectionList_[mainSocket_].DataToTransmit.size() > 0)
							{
								SendData(mainSocket_);
							}
						}
					}
					Sleep(1000);
				}
				catch (std::exception const& e)
				{
					printf("Bug StartClientConnection - Sending Data: %s\n", e.what());
				}
			}

		}
		catch (std::exception const& e)
		{
			printf("Bug StartClientConnection - Connecting: %s\n", e.what());
		}
		Sleep(1000);
	}
}






void ConnectionManager::PayloadToSendAll(Payload data)
{
	for (auto it = connectionList_.begin(); it != connectionList_.end(); it++)
	{
		it->second.DataToTransmit.push_back(data);
		//FD_SET(it->first, &writefds_);
	}
}







Connection ConnectionManager::CreateNewConnection(char *ipAddress, int port )
{
	Connection newConnection;

	newConnection.SockInfo.sin_family = AF_INET;
	newConnection.SockInfo.sin_port = htons(port);
	newConnection.SockInfo.sin_addr.s_addr = inet_addr(ipAddress);
	newConnection.FileDescriptor = mainSocket_;
	newConnection.Port = htons(newConnection.SockInfo.sin_port);
	newConnection.IPAddress = inet_ntoa(newConnection.SockInfo.sin_addr);
	newConnection.ReceivedBytes = 0;
	newConnection.ExpectedSize = 0;

	return newConnection;
}


void ConnectionManager::SetMasterDescriptor()
{
	FD_ZERO(&masterfds_);
	FD_SET(mainSocket_, &masterfds_);
	maxFileDescriptors_ = mainSocket_;
}



int ConnectionManager::AcceptConnection()
{
	int newClientSocket = 0;
	//init connnection
	Connection newClient = CreateNewConnection("",0);
	int nlen = sizeof(SOCKADDR_IN);
	memset(&newClient.SockInfo, 0, sizeof(newClient.SockInfo));
	newClientSocket = accept(mainSocket_, (SOCKADDR *)&newClient.SockInfo, &nlen);
	if (newClientSocket <= 0)
	{
		char errBuff[100];
		strerror_s(errBuff, errno);
		printf("Socket accept Failed: %d - %s.\n", newClientSocket, errBuff);
	}
	newClient.FileDescriptor = newClientSocket;
	newClient.Port = htons(newClient.SockInfo.sin_port);
	newClient.IPAddress = inet_ntoa(newClient.SockInfo.sin_addr);
	if (connectionList_.count(newClientSocket))
	{
		//TODO: Free ipaddress, and whatever else may have been allocated memory.
		printf("Reusing Socket File Descriptor...Should check for leaks in this struct\n");
		printf("Old Connection: %d - %s - %d\n", newClientSocket, connectionList_[newClientSocket].IPAddress, connectionList_[newClientSocket].Port);
	}
	printf("New Connection: %d - %s - %d\n", newClientSocket, newClient.IPAddress, newClient.Port);
	connectionList_[newClientSocket] = newClient;
	
	return newClientSocket;

}


int ConnectionManager::ReceiveExpectedSize(int currentSocketDescriptor)
{
	//Read size first
	//Might need a select to watch descriptor? but how to send :O
	char expectedSizeResponse[16];
	auto result = recv(currentSocketDescriptor, expectedSizeResponse, 16, 0);
	if (result == 0)
	{
		//clean close conn
		printf("%d - %s - Socket close recv 1.\n", currentSocketDescriptor, connectionList_[currentSocketDescriptor].IPAddress);
	}
	else if (result < 0)
	{
		char errBuff[100];
		strerror_s(errBuff, errno);
		printf("%d - %s - Socket recv Failed 1: %d - %s.\n", currentSocketDescriptor, connectionList_[currentSocketDescriptor].IPAddress, result, errBuff);
	}
	else
	{
		connectionList_[currentSocketDescriptor].ExpectedSize = atoi(expectedSizeResponse);
		printf("%d - %s - Expected Bytes: %i\n", currentSocketDescriptor, connectionList_[currentSocketDescriptor].IPAddress, connectionList_[currentSocketDescriptor].ExpectedSize);
		connectionList_[currentSocketDescriptor].ExpectedData = (char*)malloc(connectionList_[currentSocketDescriptor].ExpectedSize + 1);
	}
	return result;
}


int ConnectionManager::ReceiveExpectedData(int currentSocketDescriptor)
{
	//if size return. get actual data
	//attempt to get payload if its ready otherwise reloop around
	auto bytesIn = recv(currentSocketDescriptor,
		connectionList_[currentSocketDescriptor].ExpectedData + connectionList_[currentSocketDescriptor].ReceivedBytes,
		connectionList_[currentSocketDescriptor].ExpectedSize - connectionList_[currentSocketDescriptor].ReceivedBytes, 0);

	//If its ready append the bytes and loop aaround again for more bytes if we didnt get the full message.
	if (bytesIn >= 0)
	{
		printf("%d - %s - Byte Segment: %i\n", currentSocketDescriptor, connectionList_[currentSocketDescriptor].IPAddress, bytesIn);
		connectionList_[currentSocketDescriptor].ReceivedBytes += bytesIn;

		if (connectionList_[currentSocketDescriptor].ExpectedSize == connectionList_[currentSocketDescriptor].ReceivedBytes)
		{
			Payload d;
			std::string s(connectionList_[currentSocketDescriptor].ExpectedData, connectionList_[currentSocketDescriptor].ExpectedSize);
			std::stringstream is(s);
			{
				cereal::BinaryInputArchive iarchive(is);
				iarchive(d);
			}
			printf("%d - %s - Data: \"%s\"\n", currentSocketDescriptor, connectionList_[currentSocketDescriptor].IPAddress, d.data.c_str());

			//CLEAN UP connection buffer and size counts
			connectionList_[currentSocketDescriptor].ExpectedSize = 0;
			connectionList_[currentSocketDescriptor].ReceivedBytes = 0;
			free(connectionList_[currentSocketDescriptor].ExpectedData);
		}
	}
	return bytesIn;
}




void ConnectionManager::SendData(int currentSocketDescriptor)
{
	std::stringstream ss;
	auto data = connectionList_[currentSocketDescriptor].DataToTransmit[0];
	connectionList_[currentSocketDescriptor].DataToTransmit.pop_front();
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(data);
	}
	auto stringToSend = ss.str();
	auto charToSend = stringToSend.c_str();
	if (strcmp(charToSend, "") != 0)
	{
		char payloadSize[16];
		sprintf_s(payloadSize, sizeof(payloadSize), "%d", stringToSend.size());
		int bytesSent = send(currentSocketDescriptor, payloadSize, sizeof(payloadSize), 0);
		if (bytesSent == SOCKET_ERROR)
		{
			printf("Error: %ld - %ld.\n", currentSocketDescriptor, WSAGetLastError());
			return;
			//check iresult ? or hsocket? for status or try reopen/connect
		}
		bytesSent = send(currentSocketDescriptor, charToSend, stringToSend.size(), 0);
		if (bytesSent == SOCKET_ERROR)
		{
			printf("Error: %ld - %ld.\n", currentSocketDescriptor, WSAGetLastError());
			return;
		}
		else
		{
			printf("Bytes: %ld\n", bytesSent);
			printf("Data: \"%s\"\n", data.data.c_str());
		}
	}
}






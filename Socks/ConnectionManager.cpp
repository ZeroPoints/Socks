
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

	//maybe dont use tcp use udp?

	auto currentSocketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (currentSocketDescriptor == INVALID_SOCKET)
	{
		printf("Socket Init Failed.\n");
		return;
	}

	sockAddr_.sin_family = AF_INET;
	sockAddr_.sin_port = htons(5001);
	//will this mean i cant use external ip or ipv6?
	sockAddr_.sin_addr.s_addr = htonl(INADDR_ANY);

	//reuse socket descriptor
	result = setsockopt(currentSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, (char *)&flagOn, sizeof(flagOn));
	if (result < 0)
	{
		printf("Socket setsockopt Failed.\n");
		return;
	}

	//Non Blocking code
	result = ioctlsocket(currentSocketDescriptor, FIONBIO, &nonBlocking);
	if (result < 0)
	{
		printf("Socket ioctl Failed.\n");
		return;
	}

	//generic bind statement
	result = bind(currentSocketDescriptor, (SOCKADDR *)(&sockAddr_), sizeof(sockAddr_));
	if (result == SOCKET_ERROR)
	{
		printf("Socket bind Failed.\n");
		return;
	}

	//does the value show how many concurrent descriptors i can run?
	result = listen(currentSocketDescriptor, 16);
	if (result == SOCKET_ERROR)
	{
		printf("Socket listen Failed.\n");
		return;
	}

	fd_set masterfds, currentfds;
	FD_ZERO(&masterfds);
	FD_SET(currentSocketDescriptor, &masterfds);
	int sin_size = sizeof(sockaddr_in);
	int maxFileDescriptors = currentSocketDescriptor;
	char errBuff[100];
	int newClientSocket = 0;
	while (true)
	{
		try
		{
			memcpy(&currentfds, &masterfds, sizeof(masterfds));
			result = select(maxFileDescriptors + 1, &currentfds, NULL, NULL, NULL);
			if (result < 0)
			{
				strerror_s(errBuff, errno);
				printf("Socket select Failed: %d - %s.\n", result, errBuff);
				//return;
			}
			for (int fd = 0; fd <= maxFileDescriptors; fd++)
			{
				if (FD_ISSET(fd, &currentfds))
				{
					if (fd == currentSocketDescriptor)
					{
						newClientSocket = 0;
						//Keep grabbing any new conns if avail then go back to select
						while (newClientSocket != -1)
						{
							Connection newClient;
							int nlen = sizeof(SOCKADDR_IN);
							memset(&newClient.SockInfo, 0, sizeof(newClient.SockInfo));
							newClientSocket = accept(currentSocketDescriptor, (SOCKADDR *)&newClient.SockInfo, &nlen);
							if (newClientSocket <= 0)
							{
								strerror_s(errBuff, errno);
								printf("Socket accept Failed: %d - %s.\n", newClientSocket, errBuff);
								break;
							}
							newClient.FileDescriptor = newClientSocket;
							newClient.Port = htons(newClient.SockInfo.sin_port);
							newClient.IPAddress = inet_ntoa(newClient.SockInfo.sin_addr);
							newClient.ReceivedBytes = 0;
							newClient.ExpectedSize = 0;
							if (connectionList_.count(newClientSocket))
							{
								//TODO: Free ipaddress, and whatever else may have been allocated memory.
								printf("Reusing Socket File Descriptor...Should check for leaks in this struct\n");
								printf("Old Connection: %d - %s - %d\n", newClientSocket, connectionList_[newClientSocket].IPAddress, connectionList_[newClientSocket].Port);
							}
							connectionList_[newClientSocket] = newClient;
							printf("New Connection: %d - %s - %d\n", newClientSocket, newClient.IPAddress, newClient.Port);
							FD_SET(newClientSocket, &masterfds);
							if (newClientSocket > maxFileDescriptors)
							{
								maxFileDescriptors = newClientSocket;
							}
						}
					}
					else
					{
						//For this connection if I already have the size then go get the data
						if (connectionList_[fd].ExpectedSize == 0)
						{
							//read size of data expected
							//TODO: Probably got to free this char
							char expectedSizeResponse[16];
							auto result = recv(fd, expectedSizeResponse, 16, 0);
							if (result == 0)
							{
								//clean close conn
								printf("%d - %s - Socket close recv 1.\n", fd, connectionList_[fd].IPAddress);
								maxFileDescriptors = CleanConnectionAndDesriptor(fd, maxFileDescriptors, masterfds);

								break;
							}
							else if (result < 0)
							{
								strerror_s(errBuff, errno);
								printf("%d - %s - Socket recv Failed 1: %d - %s.\n", fd, connectionList_[fd].IPAddress, result, errBuff);
								maxFileDescriptors = CleanConnectionAndDesriptor(fd, maxFileDescriptors, masterfds);
								break;
							}
							connectionList_[fd].ExpectedSize = atoi(expectedSizeResponse);
							printf("%d - %s - Expected Bytes: %i\n", fd, connectionList_[fd].IPAddress, connectionList_[fd].ExpectedSize);
							connectionList_[fd].ExpectedData = (char*)malloc(connectionList_[fd].ExpectedSize + 1);
						}

						//attempt to get payload if its ready otherwise reloop around
						auto bytesIn = recv(fd,
							connectionList_[fd].ExpectedData + connectionList_[fd].ReceivedBytes,
							connectionList_[fd].ExpectedSize - connectionList_[fd].ReceivedBytes, 0);

						//If its ready append the bytes and loop aaround again for more bytes if we didnt get the full message.
						if (bytesIn >= 0)
						{
							printf("%d - %s - Byte Segment: %i\n", fd, connectionList_[fd].IPAddress, bytesIn);
							connectionList_[fd].ReceivedBytes += bytesIn;

							if (connectionList_[fd].ExpectedSize == connectionList_[fd].ReceivedBytes)
							{
								Payload d;
								std::string s(connectionList_[fd].ExpectedData, connectionList_[fd].ExpectedSize);
								std::stringstream is(s);
								{
									cereal::BinaryInputArchive iarchive(is);
									iarchive(d);
								}
								printf("%d - %s - Data: \"%s\"\n", fd, connectionList_[fd].IPAddress, d.data.c_str());

								//CLEAN UP connection buffer and size counts
								connectionList_[fd].ExpectedSize = 0;
								connectionList_[fd].ReceivedBytes = 0;
								free(connectionList_[fd].ExpectedData);
							}
						}


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





int ConnectionManager::CleanConnectionAndDesriptor(int fd, int maxFileDescriptors, fd_set &masterfds)
{
	closesocket(fd);
	FD_CLR(fd, &masterfds);
	if (fd == maxFileDescriptors)
	{
		while (FD_ISSET(maxFileDescriptors, &masterfds) == FALSE)
		{
			maxFileDescriptors--;
		}
	}
	return maxFileDescriptors;
}














void ConnectionManager::StartClientConnection(char *ipAddress, int port)
{
	auto currentSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (currentSocket == INVALID_SOCKET)
	{
		printf("Socket Init Failed.");
	}
	sockAddr_.sin_family = AF_INET;
	sockAddr_.sin_port = htons(port);
	sockAddr_.sin_addr.s_addr = inet_addr(ipAddress);
	bool connected = false;
	while (!connected)
	{
		try
		{
			auto connectResult = connect(currentSocket, (SOCKADDR *)&sockAddr_, sizeof(sockAddr_));
			if (connectResult == SOCKET_ERROR)
			{
				printf("Attempt to connect failed. - %ld \n", WSAGetLastError());
				//clean up current socket
				connected = false;
				Sleep(5000);
				continue;
			}
			connected = true;
			printf("IP: %s\n", inet_ntoa(sockAddr_.sin_addr));
			printf("Port: %d\n", htons(sockAddr_.sin_port));

			while (true)
			{
				try
				{
					if (dataToTransmit_.size() > 0)
					{
						std::stringstream ss;
						auto data = dataToTransmit_[0];
						dataToTransmit_.pop_front();
						{
							cereal::BinaryOutputArchive oarchive(ss);
							oarchive(data);
						}
						auto stringToSend = ss.str();
						auto charToSend = stringToSend.c_str();
						if (strcmp(charToSend, "") != 0)
						{
							SOCKADDR_IN thisSenderInfo;
							char payloadSize[16];
							sprintf_s(payloadSize, sizeof(payloadSize), "%d", stringToSend.size());
							printf("Client: %ld\n", currentSocket);
							memset(&thisSenderInfo, 0, sizeof(thisSenderInfo));
							int nlen = sizeof(thisSenderInfo);
							getsockname(currentSocket, (SOCKADDR *)&thisSenderInfo, &nlen);
							printf("IP: %s\n", inet_ntoa(thisSenderInfo.sin_addr));
							printf("Port: %d\n", htons(thisSenderInfo.sin_port));
							int bytesSent = send(currentSocket, payloadSize, sizeof(payloadSize), 0);
							if (bytesSent == SOCKET_ERROR)
							{
								printf("Error: %ld - %ld.\n", currentSocket, WSAGetLastError());
								return;
								//check iresult ? or hsocket? for status or try reopen/connect
							}
							bytesSent = send(currentSocket, charToSend, stringToSend.size(), 0);
							if (bytesSent == SOCKET_ERROR)
							{
								printf("Error: %ld - %ld.\n", currentSocket, WSAGetLastError());
								return;
							}
							else
							{
								printf("Bytes: %ld\n", bytesSent);
								printf("Data: \"%s\"\n", data.data.c_str());
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



void ConnectionManager::PayloadToSend(Payload data)
{
	dataToTransmit_.push_back(data);
}







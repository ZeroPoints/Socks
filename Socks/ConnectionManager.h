#pragma once




#ifndef __CONNECTIONMANAGER_H__ 
#define __CONNECTIONMANAGER_H__  



#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <winsock.h>
#include <map>
#include "Payload.h"
#include "Connection.h"
#include <deque>


#pragma comment(lib,"WS2_32")



class ConnectionManager
{

public:

	ConnectionManager();
	~ConnectionManager();

	void StartServerListener();

	int CleanConnectionAndDesriptor(int fd);

	void StartClientConnection(char *ipAddress, int port);
	void PayloadToSend(Payload data);
	void PayloadToSendAll(Payload data);

	int AcceptConnection();
	void SendData(int currentSocketDescriptor);
	int ReceiveExpectedSize(int currentSocketDescriptor);
	int ReceiveExpectedData(int currentSocketDescriptor);

	void SetMasterDescriptor();
	Connection CreateNewConnection(char *ipAddress, int port);


private:

	SOCKADDR_IN sockAddr_;
	SOCKET mainSocket_;

	//std::deque<Payload> dataToTransmit_;
	std::map<int, Connection> connectionList_;


	int maxFileDescriptors_;
	fd_set masterfds_, readfds_, writefds_;

};


#endif 





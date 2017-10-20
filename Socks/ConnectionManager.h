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

	int CleanConnectionAndDesriptor(int fd, int maxFileDescriptors, fd_set &masterfds);

	void StartClientConnection(char *ipAddress, int port);
	void PayloadToSend(Payload data);

private:

	sockaddr_in sockAddr_;
	SOCKET currentSocket_;

	std::deque<Payload> dataToTransmit_;
	std::map<int, Connection> connectionList_;

};


#endif 





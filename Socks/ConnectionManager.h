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

	/*
	Inits stuff for this object
	*/
	ConnectionManager();

	/*
	Cleans up socket stuff...
	*/
	~ConnectionManager();


	


	/*
	Starts a server listener infinite loop
	*/
	void StartServerListener();



	/*
	Should maybe clean up the connectionList_
	For now i wont just so i can see debug of who has connected in whole runtime.(which should maybe be logged instead)
	*/
	int CleanConnectionAndDesriptor(int fd);





	/*
	Starts a client connection
	*/
	void StartClientConnection(char *ipAddress, int port);

	/*
	Sends the payload to all connections in the connectionlist
	*/
	void PayloadToSendAll(Payload data);
	void PayloadToSend(Payload data);


	/*
	Accepts a connection and adds it to the connectionlist
	*/
	int AcceptConnection();

	/*
	Sends any data in the DataToTransmit buffer
	*/
	void SendData(int currentSocketDescriptor);


	/*
	Gets the expected size of data about to be received
	*/
	int ReceiveExpectedSize(int currentSocketDescriptor);

	/*
	Receives the data from the socket
	*/
	int ReceiveExpectedData(int currentSocketDescriptor);


	/*
	Sets master stuff
	*/
	void SetMasterDescriptor();


	/*
	Creates a new socket connection
	*/
	Connection CreateNewConnection(char *ipAddress, int port);


private:

	SOCKADDR_IN sockAddr_;
	SOCKET mainSocket_;

	std::map<int, Connection> connectionList_;

	int maxFileDescriptors_;
	fd_set masterfds_, readfds_, writefds_;

};


#endif 





#pragma once

#include <winsock.h>



struct Connection
{

	int FileDescriptor;

	SOCKADDR_IN SockInfo;

	u_short Port;

	char* IPAddress;


	//Incoming payload size
	int ExpectedSize;

	//Size that has come in currently
	int ReceivedBytes;

	//Payload buffer for data coming in
	char* ExpectedData;


	  
};
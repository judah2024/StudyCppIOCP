#pragma once

#include "IOCPServer.h"

class EchoServer
{
public:
	EchoServer();

	bool Run(int portNumber, const UINT32 maxClientCount);
	void Stop();

private:
	void OnConnect(const UINT32 clientID);
	void OnDisconnect(const UINT32 clientID);
	void OnReceive(const UINT32 clientID, const char* data, const UINT32 size);

private:
	IOCPServer _network;
};


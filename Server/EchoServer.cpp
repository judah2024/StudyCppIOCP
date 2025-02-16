#include "EchoServer.h"

EchoServer::EchoServer()
{
	_network.SetOnConnect(std::bind(&EchoServer::OnConnect, this, std::placeholders::_1));
	_network.SetOnDisconnect(std::bind(&EchoServer::OnDisconnect, this, std::placeholders::_1));
	_network.SetOnReceive(std::bind(&EchoServer::OnReceive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

bool EchoServer::Run(int portNumber, const UINT32 maxClientCount)
{
	bool result = _network.InitSocket();
	if (result == false)
	{
		printf("[����] ���� �ʱ�ȭ ����\n");
		return false;
	}

	result = _network.BindAndListen(portNumber);
	if (result == false)
	{
		printf("[����] ���� ��� ����\n");
		return false;
	}

	result = _network.StartServer(maxClientCount);
	if (result == false)
	{
		printf("[����] ���� ���� ����\n");
		return false;
	}

	return true;
}

void EchoServer::Stop()
{
	_network.DestroyThread();
}

void EchoServer::OnConnect(const UINT32 clientID)
{
	printf("[OnConnect] Ŭ���̾�Ʈ : ID(%d)\n", clientID);
}

void EchoServer::OnDisconnect(const UINT32 clientID)
{
	printf("[OnDisconnect] Ŭ���̾�Ʈ : ID(%d)\n", clientID);
}

void EchoServer::OnReceive(const UINT32 clientID, const char* data, const UINT32 size)
{
	printf("[OnReceive] Ŭ���̾�Ʈ : ID(%d),\t dataSize(%d)\n", clientID, size);
	_network.Send(clientID, data, size);
}

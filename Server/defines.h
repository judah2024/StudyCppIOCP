#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <functional>


#define MAX_BUFFER_SIZE 1024
#define MAX_WORKERS		4

enum class IOOperation
{
	RECV,
	SEND,
};

struct stOverlappedEx
{
	WSAOVERLAPPED	overlapped;
	SOCKET			socketClient;
	WSABUF			wsaBuf;
	IOOperation		operation;
};

struct stClientInfo
{
	SOCKET			socketClient;
	stOverlappedEx	recvOverlappedEx;
	stOverlappedEx	sendOverlappedEx;
	UINT32			id;

	char			sendBuf[MAX_BUFFER_SIZE];
	char			recvBuf[MAX_BUFFER_SIZE];


	stClientInfo()
		: socketClient{ INVALID_SOCKET }
		, sendBuf{ 0 }
		, recvBuf{ 0 }
		, id{ ULONG_MAX }
	{
		ZeroMemory(&recvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&sendOverlappedEx, sizeof(stOverlappedEx));
	}
};
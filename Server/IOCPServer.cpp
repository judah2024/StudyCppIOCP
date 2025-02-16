#include "IOCPServer.h"

IOCPServer::IOCPServer()
	: mListenSocket(INVALID_SOCKET)
	, mClientCnt(0)
	, mIOCPHandle(NULL)
	, mIsWorkerRun(true)
	, mIsAccepterRun(true)
{
}

IOCPServer::~IOCPServer()
{
	WSACleanup();
}

bool IOCPServer::InitSocket()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		printf("[에러] WSAStartup() 함수 실패: %d\n", WSAGetLastError());
		return false;
	}

	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (mListenSocket == INVALID_SOCKET)
	{
		printf("[에러] socket() 함수 실패: %d\n", WSAGetLastError());
		return false;
	}

	printf("소켓 초기화 성공\n");
	return true;
}


bool IOCPServer::BindAndListen(int portNumber)
{
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNumber);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int result = bind(mListenSocket, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR_IN));
	if (result != 0)
	{
		printf("[에러] bind 함수 실패: %d\n", WSAGetLastError());
		return false;
	}

	result = listen(mListenSocket, 5);
	if (result != 0)
	{
		printf("[에러] listen() 함수 실패: %d\n", WSAGetLastError());
		return false;
	}

	printf("서버 등록 성공...\n");
	return true;
}

bool IOCPServer::StartServer(const UINT32 maxClientCount)
{
	CreateClient(maxClientCount);

	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERS);
	if (mIOCPHandle == NULL)
	{
		printf("[에러] CreateIoCompletionPort() 함수 실패: %d\n", WSAGetLastError());
		return false;
	}

	bool result = CreateWorkerThread();
	if (result == false)
	{
		return false;
	}

	result = CreateAccepterThread();
	if (result == false)
	{
		return false;
	}

	printf("서버 시작!\n");
	return true;
}

void IOCPServer::DestroyThread()
{
	mIsWorkerRun = false;
	CloseHandle(mIOCPHandle);

	for (auto& th : mIOWorkerThreads)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	mIsAccepterRun = false;
	closesocket(mListenSocket);

	if (mAccepterThread.joinable())
	{
		mAccepterThread.join();
	}

}


void IOCPServer::Send(UINT32 clientID, const char* pMsg, int nLen)
{
	SendMsg(&mClientInfos[clientID], pMsg, nLen);
}

bool IOCPServer::CreateWorkerThread()
{
	for (UINT32 i = 0; i < MAX_WORKERS; i++)
	{
		mIOWorkerThreads.emplace_back([this]() { WorkerThread(); });
	}

	printf("WorkerThread 시작...\n");
	return true;
}

bool IOCPServer::CreateAccepterThread()
{
	mAccepterThread = std::thread([this]() { AccepterThread(); });

	printf("AccepterThread 시작...\n");
	return true;
}

bool IOCPServer::BindIOCP(stClientInfo* pClientInfo)
{
	auto hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->socketClient, mIOCPHandle, (ULONG_PTR)pClientInfo, 0);

	if (hIOCP == NULL || mIOCPHandle != hIOCP)
	{
		printf("[에러] CreateIoCompletionPort() 함수 실패: %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

bool IOCPServer::BindRecv(stClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	pClientInfo->recvOverlappedEx.wsaBuf.len = MAX_BUFFER_SIZE;
	pClientInfo->recvOverlappedEx.wsaBuf.buf = pClientInfo->recvBuf;
	pClientInfo->recvOverlappedEx.operation = IOOperation::RECV;

	int result = WSARecv(pClientInfo->socketClient, 
		&(pClientInfo->recvOverlappedEx.wsaBuf), 
		1,
		&dwRecvNumBytes,
		&dwFlag,
		(LPWSAOVERLAPPED)&(pClientInfo->recvOverlappedEx),
		NULL);

	if (result == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[에러] WSARecv() 함수 실패: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool IOCPServer::SendMsg(stClientInfo* pClientInfo,const char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;

	CopyMemory(pClientInfo->sendBuf, pMsg, nLen);

	pClientInfo->sendOverlappedEx.wsaBuf.len = nLen;
	pClientInfo->sendOverlappedEx.wsaBuf.buf = pClientInfo->sendBuf;
	pClientInfo->sendOverlappedEx.operation = IOOperation::SEND;

	int result = WSASend(pClientInfo->socketClient,
		&(pClientInfo->sendOverlappedEx.wsaBuf),
		1,
		&dwRecvNumBytes,
		0,
		(LPWSAOVERLAPPED) & (pClientInfo->sendOverlappedEx),
		NULL);

	if (result == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[에러] WSASend() 함수 실패: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void IOCPServer::WorkerThread()
{
	stClientInfo* pClientInfo = nullptr;
	BOOL bSuccess = TRUE;
	DWORD dwIoSize = 0;
	LPOVERLAPPED lpOverlapped = nullptr;

	while (mIsWorkerRun)
	{
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle,
			&dwIoSize,
			(PULONG_PTR)&pClientInfo,
			&lpOverlapped,
			INFINITE);

		if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == NULL)
		{
			mIsWorkerRun = false;
			continue;
		}

		if (lpOverlapped == NULL)
		{
			continue;
		}

		if (bSuccess == FALSE || (dwIoSize == 0 && bSuccess == TRUE))
		{
			CloseSocket(pClientInfo);
			continue;
		}

		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

		switch (pOverlappedEx->operation)
		{
		case IOOperation::RECV:
			mOnReceive(pClientInfo->id, pClientInfo->recvBuf, dwIoSize);
			BindRecv(pClientInfo);
			break;
		case IOOperation::SEND:
			printf("[송신] bytes: %d,\t msg: %s\n", dwIoSize, pClientInfo->sendBuf);
			break;
		default:
			printf("socket(%d) 예외상황\n", (int)pClientInfo->socketClient);
			break;
		}
	}
}

void IOCPServer::AccepterThread()
{
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);

	while (mIsAccepterRun)
	{
		stClientInfo* pClientInfo = GetEmptyClientInfo();
		if (pClientInfo == NULL)
		{
			printf("[에러] Client Full\n");
			return;
		}

		pClientInfo->socketClient = accept(mListenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (pClientInfo->socketClient == INVALID_SOCKET)
		{
			continue;
		}

		bool result = BindIOCP(pClientInfo);
		if (result == false)
		{
			return;
		}

		result = BindRecv(pClientInfo);
		if (result == false)
		{
			return;
		}

		// TODO : OnConnect
		char clientIP[32] = { 0 };
		inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, 32 - 1);
		mOnConnect(pClientInfo->id);

		++mClientCnt;
	}
}

void IOCPServer::CloseSocket(stClientInfo* pClientInfo, bool bIsForce)
{
	struct linger stLinger = { 0, 0 };
	if (bIsForce == true)
	{
		stLinger.l_onoff = 1;
	}

	shutdown(pClientInfo->socketClient, SD_BOTH);
	setsockopt(pClientInfo->socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	closesocket(pClientInfo->socketClient);
	pClientInfo->socketClient = INVALID_SOCKET;

	mOnDisconnect(pClientInfo->id);
}

void IOCPServer::CreateClient(const UINT32 maxClientCount)
{
	for (auto i = 0; i < maxClientCount; i++)
	{
		mClientInfos.emplace_back();
		mClientInfos[i].id = i;
	}
}

stClientInfo* IOCPServer::GetEmptyClientInfo()
{
	for (auto& client : mClientInfos)
	{
		if (client.socketClient == INVALID_SOCKET)
		{
			return &client;
		}
	}

	return nullptr;
}

#pragma once

#include "defines.h"

class IOCPServer
{
private:
	using ReceiveCallback = std::function<void(const UINT32, const char*, const UINT32)>;
	using ConnectionCallback = std::function<void(const UINT32)>;

private:
	std::vector<stClientInfo>	mClientInfos;
	SOCKET						mListenSocket;
	int							mClientCnt;
	std::vector<std::thread>	mIOWorkerThreads;
	std::thread					mAccepterThread;
	HANDLE						mIOCPHandle;
	bool						mIsWorkerRun;
	bool						mIsAccepterRun;

	ConnectionCallback mOnConnect;
	ConnectionCallback mOnDisconnect;
	ReceiveCallback mOnReceive;

public:
	IOCPServer();
	~IOCPServer();


	/// <summary> IPv4, TCP 프로토콜로 초기화. </summary>
	bool InitSocket();

	bool BindAndListen(int portNumber);
	bool StartServer(const UINT32 maxClientCount);

	void Send(UINT32 clientID, const char* pMsg, int nLen);
	void DestroyThread();

	void SetOnConnect(ConnectionCallback callback) { mOnConnect = callback; }
	void SetOnDisconnect(ConnectionCallback callback) { mOnDisconnect = callback; }
	void SetOnReceive(ReceiveCallback callback) { mOnReceive = callback; }

private:
	bool CreateWorkerThread();
	bool CreateAccepterThread();
	
	bool BindIOCP(stClientInfo* pClientInfo);
	bool BindRecv(stClientInfo* pClientInfo);
	bool SendMsg(stClientInfo* pClientInfo, const char* pMsg, int nLen);

	void WorkerThread();
	void AccepterThread();
	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false);

	void CreateClient(const UINT32 maxClientCount);
	stClientInfo* GetEmptyClientInfo();
};
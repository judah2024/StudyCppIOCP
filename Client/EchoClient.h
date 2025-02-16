#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class EchoClient {
private:
    SOCKET clientSocket;
    bool isConnected;

public:
    EchoClient() : clientSocket(INVALID_SOCKET), isConnected(false) {}

    bool Initialize() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }

        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            return false;
        }

        return true;
    }

    bool Connect(const char* serverIP, const char* port) {
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);
        serverAddr.sin_port = htons(static_cast<u_short>(std::stoi(port)));

        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed" << std::endl;
            return false;
        }

        isConnected = true;
        return true;
    }

    void Run() {
        std::string message;
        char buffer[1024];

        while (isConnected) {
            std::cout << "Enter message (or 'quit' to exit): ";
            std::getline(std::cin, message);

            if (message == "quit") {
                break;
            }

            // Send message
            if (send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0) == SOCKET_ERROR) {
                std::cerr << "Send failed" << std::endl;
                break;
            }

            // Receive echo
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived == SOCKET_ERROR) {
                std::cerr << "Receive failed" << std::endl;
                break;
            }

            buffer[bytesReceived] = '\0';
            std::cout << "Server echo: " << buffer << std::endl;
        }
    }

    void Cleanup() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
        WSACleanup();
    }

    ~EchoClient() {
        Cleanup();
    }
};


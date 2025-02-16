#include "EchoClient.h"

int main() {
    EchoClient client;

    if (!client.Initialize()) {
        std::cerr << "Client initialization failed" << std::endl;
        return 1;
    }

    if (!client.Connect("127.0.0.1", "12345")) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    std::cout << "Connected to server" << std::endl;
    client.Run();

    return 0;
}
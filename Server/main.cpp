#include "EchoServer.h"

constexpr UINT16 SERVER_PORT = 12345;
constexpr UINT16 MAX_CLIENT = 100;
const std::string quitString = "quit";

int main()
{
	EchoServer server;

	bool result = server.Run(SERVER_PORT, MAX_CLIENT);
	if (result == false)
	{
		return 1;
	}

	printf("서버가 시작되었습니다. 종료하려면 %s를 입력하세요.\n", quitString.data());
	while (true)
	{
		std::string sInput;
		std::getline(std::cin, sInput);
		if (sInput == quitString)
		{
			break;
		}
	}

	server.Stop();
	return 0;
}
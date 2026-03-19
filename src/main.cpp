#include "Server.hpp"
#include <iostream>
#include <csignal>

// volatile 컴파일러 최적화 방지, 매번 메모리에서 값을 읽도록 강제
// sig_atomic_t  시그널 핸들러에서 안전하게 읽고 쓸 수 있는 정수 타입
volatile sig_atomic_t g_running = 1;

void	signalhanlder(int signum)
{
	(void) signum;
	g_running = 0;
}

// ./ircserv <port> <password>
int main(int argc, char **argv) {

	signal(SIGINT, signalhanlder);
	signal(SIGTERM, signalhanlder);

	if (argc != 3) {
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	const char* port = argv[1];
	const char* password = argv[2];

	Server server(port, password);
	if (!server.initialize()) {
		std::cerr << "Failed to initialize server" << std::endl;
		return 1;
	}

	server.run();

	return 0;
}

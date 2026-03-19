#include "Bot.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>

bool g_stop = false;

void handle_sigint(int sig) {
	(void)sig;
	g_stop = true;
}

int main(int argc, char **argv) {
	if (argc != 4) {
		std::cerr << "사용법: ./ircbot <서버IP> <포트> <비밀번호>" << std::endl;
		std::cerr << "예시: ./ircbot localhost(또는 127.0.0.1) <포트> <비밀번호>" << std::endl;
		return 1;
	}

	std::string ip = argv[1];
	int port = std::atoi(argv[2]);
	std::string password = argv[3];

	std::string nickname = "ircbot";

	if (port <= 0 || port > 65535) {
		std::cerr << "에러: 유효하지 않은 포트 번호. (1 ~ 65535)" << std::endl;
		return 1;
	}

	struct sigaction sa;
	sa.sa_handler = handle_sigint;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	try {
		Bot bot(ip, port, password, nickname);
		bot.run();
	} catch (const std::exception &e) {
		std::cerr << "[Bot] 에러로 인한 봇 종료: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "[Bot] 봇 종료";

	return 0;
}

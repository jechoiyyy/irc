#include "Bot.hpp"
#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cerrno>
#include <stdexcept>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <cstdlib>
#include <ctime>
#include <sstream>

extern bool g_stop;

Bot::Bot(const std::string& ip, int port, const std::string& pass, const std::string& nick)
	: _sock(-1)
	, _server_ip(ip)
	, _port(port)
	, _password(pass)
	, _nickname(nick) {
	std::cout << "[Bot] 봇 초기화 시작. 대상: " << _server_ip << ":" << _port << std::endl;
}

Bot::~Bot() {
	if (_sock != -1) {
		std::string quit_msg = "QUIT :봇 종료\r\n";
		send(_sock, quit_msg.c_str(), quit_msg.length(), 0);

		close(_sock);
		_sock = -1;
		std::cout << "[Bot] 봇 종료.";
	}
}

void Bot::connectServer() {
	_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock < 0) {
		throw std::runtime_error("소켓 생성 실패");
	}

	int flags = fcntl(_sock, F_GETFL, 0);
	if (flags < 0 || fcntl(_sock, F_SETFL, flags | O_NONBLOCK) < 0) {
		throw std::runtime_error("소켓 논블로킹 설정 실패");
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(_port);

	struct hostent *host_info = gethostbyname(_server_ip.c_str());
	if (host_info == NULL) {
		throw std::runtime_error("잘못된 호스트 이름 또는 IP 주소");
	}
	server_addr.sin_addr.s_addr = *((unsigned long *)host_info->h_addr_list[0]);

	if (connect(_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		if (errno != EINPROGRESS) {
			throw std::runtime_error("서버 연결 실패");
		}
	}
}

void Bot::authenticate() {
	std::string auth_msg;

	if (!_password.empty()) {
		auth_msg += "PASS " + _password + "\r\n";
	}

	auth_msg += "NICK " + _nickname + "\r\n";
	auth_msg += "USER " + _nickname + " 0 * :" + _nickname + " Bot\r\n";

	_write_buf += auth_msg;
}

void Bot::receiveData() {
	char buf[1024];

	int bytes_read = recv(_sock, buf, sizeof(buf) - 1, 0);

	if (bytes_read > 0) {
		buf[bytes_read] = '\0';
		_read_buf += buf;
	} else if (bytes_read == 0) {
		// 서버에서 연결 종료한 경우
		throw std::runtime_error("서버에 의해 연결 종료");
	} else {
		// 논블로킹 상태에서 읽을 데이터 없는데 호출되는 경우
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			throw std::runtime_error("데이터 수신 중 에러 발생");
		}
	}
}

void Bot::sendData() {
	if (_write_buf.empty()) {
		return ;
	}

	int bytes_sent = send(_sock, _write_buf.c_str(), _write_buf.length(), 0);

	if (bytes_sent > 0) {
		_write_buf.erase(0, bytes_sent);
	} else if (bytes_sent < 0) {
		// 논블로킹 상태에서 읽을 데이터 없는데 호출되는 경우
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			throw std::runtime_error("데이터 수신 중 에러 발생");
		}
	}
}

void Bot::extractLine() {
	size_t pos;

	while ((pos = _read_buf.find("\r\n")) != std::string::npos) {
		std::string raw_msg = _read_buf.substr(0, pos);

		_read_buf.erase(0, pos + 2);

		if (!raw_msg.empty()) {
			processMessage(raw_msg);
		}
	}
}

void Bot::processMessage(const std::string &raw_msg) {
	std::cout << "[Bot Recv] " << raw_msg << std::endl;

	if (raw_msg.find(" PRIVMSG ") != std::string::npos) {
		handlePrivmsg(raw_msg);
	} else if (raw_msg.find(" INVITE ") != std::string::npos) {
		handleInvite(raw_msg);
	}
}

void Bot::sendPrivmsg(const std::string &target, const std::string &msg) {
	std::string packet = "PRIVMSG " + target + " :" + msg + "\r\n";
	_write_buf += packet;

	std::cout << "[Bot Send] PRIVMSG -> " << target << " : " << msg << std::endl;
}

void	Bot::joinChannel(const std::string &channel) {
	std::string join_msg = "JOIN " + channel + "\r\n";
	_write_buf += join_msg;
	std::cout << "[Bot] 채널 입장 시도: " << channel << std::endl;
}

void Bot::handlePrivmsg(const std::string &msg) {
	if (msg[0] != ':')
		return ;

	// 닉네임 추출
	size_t excl_pos = msg.find('!');
	if (excl_pos == std::string::npos)
		return ;
	std::string sender = msg.substr(1, excl_pos - 1);

	// 메시지 내용 추출
	size_t privmsg_pos = msg.find(" PRIVMSG ");
	size_t target_start = privmsg_pos + 9;
	size_t target_end = msg.find(" :", target_start);
	std::string target = msg.substr(target_start, target_end - target_start);

	std::string text = msg.substr(target_end + 2);
	size_t last = text.find_last_not_of("\r\n");
	if (last != std::string::npos) {
		text = text.substr(0, last + 1);
	}

	std::string receiver = (target[0] == '#') ? target : sender;

	// 명령어 처리
	if (text == "!help") {
		sendPrivmsg(receiver, "사용 가능한 명령어: !help, !ping, !hello");
	} else if (text == "!ping") {
		sendPrivmsg(receiver, "pong!");
	} else if (text == "!hello" || text == "!hi") {
		sendPrivmsg(receiver, sender + "님, 환영합니다.");
	} else if (text == "!dice") {
		std::srand(std::time(NULL));
		int result = (std::rand() % 6) + 1;

		std::stringstream ss;
		ss << "주사위 결과: " << result;
		sendPrivmsg(receiver, ss.str());
	} else if (text == "!time") {
		std::time_t now = std::time(NULL);
		char buf[80];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
		sendPrivmsg(receiver, std::string("현재 서버 시간: ") + buf);
	}
}

void Bot::handleInvite(const std::string &msg) {
	std::string trimmed = msg;
	size_t last_cmd = msg.find_last_not_of("\r\n");
	if (last_cmd != std::string::npos) {
		trimmed = trimmed.substr(0, last_cmd + 1);
	}

	size_t last_space = trimmed.find_last_of(' ');
	if (last_space != std::string::npos) {
		std::string channel = msg.substr(last_space + 1);

		if (channel[0] == ':') {
			channel = channel.substr(1);
		}

		joinChannel(channel);
	}
}

void Bot::run() {
	try {
		std::cout << "[Bot] 서버 연결 시도 ..." << std::endl;
		// 소켓 생성 및 서버 연결
		connectServer();
		// 서버 접속 후 인증 절차
		authenticate();
		std::cout << "[Bot] 서버 연결 및 인증 완료" << std::endl;
	} catch (const std::exception &e) {
		if (_sock != -1) {
			close(_sock);
		}
		std::cerr << "[Bot] 초기화 실패: " << e.what() << std::endl;
		throw ;
	}

	struct pollfd pfd;
	pfd.fd = _sock;

	std::cout << "[Bot] 이벤트 루프 시작" << std::endl;

	while (!g_stop) {
		pfd.events = POLLIN;

		if (!_write_buf.empty()) {
			pfd.events |= POLLOUT;
		}

		int ret = poll(&pfd, 1, 100);

		if (ret < 0) {
			if (errno == EINTR)
				continue ;
			std::cerr << "[Bot] poll() 에러 발생. 루프 종료." << std::endl;
			break ;
		}

		if (pfd.revents & POLLIN) {
			receiveData();
			extractLine();
		}

		if (pfd.revents & POLLOUT) {
			sendData();
		}
	}
}

#ifndef BOT_HPP
#define BOT_HPP

#include <string>

class Bot {
private:
	int			_sock;
	std::string	_server_ip;
	int			_port;
	std::string	_password;
	std::string	_nickname;

	std::string	_read_buf;
	std::string	_write_buf;

	void	connectServer();
	void	authenticate();
	void	receiveData();
	void	sendData();

	void	extractLine();
	void	processMessage(const std::string &raw_msg);

	void	sendPrivmsg(const std::string &target, const std::string &msg);

	void	joinChannel(const std::string &channel);

	void	handlePrivmsg(const std::string &msg);
	void	handleInvite(const std::string &msg);

public:
	Bot(const std::string& ip, int port, const std::string& pass, const std::string& nick);
	~Bot();

	void	run();
};

#endif

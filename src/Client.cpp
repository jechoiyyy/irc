#include "Client.hpp"
#include <cstdlib>
#include <iostream>
#include <cerrno>

Client::Client(): fd(-1), pass_registered(false), is_disconnecting(false), quit_reason("") {}

Client::Client(int fd): fd(fd), pass_registered(false), is_disconnecting(false), quit_reason("") {}


bool	Client::handle_recv()
{
	char temp[1024];
	ssize_t n = ::recv(fd, temp, sizeof(temp), 0);

	if (n > 0) {
		read_buffer.append(temp, n);
		return true;
	} else if (n == 0) {
		std::cout << "Client fd " << fd << " disconnected" << std::endl;
		return false;
	} else {
		// error
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return true;
		}
		std::cerr << "recv error: " << strerror(errno) << std::endl;
		return false;
	}
}

bool	Client::handle_send()
{
	if (write_buffer.empty()) {
		return true;
	}

	ssize_t sent = ::send(fd, write_buffer.c_str(), write_buffer.size(), 0);
	if (sent > 0) {
		write_buffer.erase(0, sent);
		return true;
	} else {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return true;
		}
		std::cerr << "send error on fd " << fd << ": " << strerror(errno) << std::endl;
		return false;
	}
}

bool	Client::is_registered() const
{
	return !nickname.empty() && !username.empty();
}

bool	Client::is_pass_registered() const
{
	return pass_registered;
}


bool	Client::extractMessage(std::string& out)
{
	size_t pos = read_buffer.find("\r\n");
	if (pos == std::string::npos) {
		return false;
	}
	out = read_buffer.substr(0,pos);

	read_buffer.erase(0, pos + 2);
	return true;
}

void	Client::send_reply(const std::string& code, const std::string& msg)
{
	std::string reply = ":localhost " + code + " " + msg + "\r\n";
	write_buffer += reply;
}


// getter
int					Client::getFd() const
{
	return this->fd;
}

const std::string&	Client::getReadBuffer() const
{
	return read_buffer;
}

const std::string&	Client::getWriteBuffer() const
{
	return write_buffer;
}

const std::string&	Client::getNickname() const
{
	return nickname;
}

const std::string&	Client::getUsername() const
{
	return username;
}

const std::string&	Client::getRealname() const
{
	return realname;
}

bool				Client::hasWriteData() const
{
	return !write_buffer.empty();
}

bool				Client::isDisconnecting() const
{
	return is_disconnecting;
}

const std::string &Client::getQuitReason() const
{
	return quit_reason;
}

// setter
void	Client::setFd(int fd)
{
	this->fd = fd;
}

void	Client::setNickname(const std::string& nickname)
{
	this->nickname = nickname;
}

void	Client::setUsername(const std::string& username)
{
	this->username = username;
}

void	Client::setRealname(const std::string& realname)
{
	this->realname = realname;
}

void	Client::setPassRegistered(bool value)
{
	this->pass_registered = value;
}

void	Client::setDisconnecting(const std::string &reason)
{
	this->is_disconnecting = true;
	quit_reason = reason;
}

void	Client::appendWriteBuffer(const std::string& msg)
{
	write_buffer += msg;
	
}

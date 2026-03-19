#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <string.h>
#include <sys/socket.h>

class Client {
	private:
		int fd;
		std::string read_buffer;
		std::string write_buffer;

		std::string nickname;
		std::string username;
		std::string realname;
		bool	pass_registered;
		bool	is_disconnecting;
		std::string quit_reason;

	public:
		Client();
		Client(int fd);

		bool	handle_recv();
		bool	handle_send();
		bool	is_registered() const;
		bool	is_pass_registered() const;


		bool	extractMessage(std::string& out);
		void	send_reply(const std::string& code, const std::string& msg);

		// getter
		int					getFd() const;
		const std::string&	getReadBuffer() const;
		const std::string&	getWriteBuffer() const;
		const std::string&	getNickname() const;
		const std::string&	getUsername() const;
		const std::string&	getRealname() const;
		bool				hasWriteData() const;
		bool				isDisconnecting() const;
		const std::string &getQuitReason() const;

		// setter
		void	setFd(int fd);
		void	setNickname(const std::string& nickname);
		void	setUsername(const std::string& username);
		void	setRealname(const std::string& realname);
		void	setPassRegistered(bool value);
		void	setDisconnecting(const std::string &reason);
		void	appendWriteBuffer(const std::string& msg);
};

#endif

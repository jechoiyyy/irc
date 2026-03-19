#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <map>
#include <poll.h>

#include "Client.hpp"
#include "Message.hpp"
#include "CommandHandler.hpp"
#include "Channel.hpp"

#define BACKLOG 10
#define BUFFER_SIZE 1024

class Server {
	private:
		int	listener;
		const char* port;
		std::string password;
		std::vector<pollfd> fds;
		std::map<int, Client> clients;
		std::map<std::string, int> nicknames;
		CommandHandler cmdHandler;
		std::map<std::string, Channel> channels;
	
	public:
		Server(const char*  port, const char* password);
		~Server();

		int		getListenerSocket();
		int		makeSocketNonblocking(int fd);
		void	handleNewConnection();
		bool	handleClientEvent(size_t index);

		void	processMessage(Client& client, const std::string& message);
		void	closeClient(size_t index);

		bool	initialize();
		void	run();

		// Channel management
		Channel* getChannel(const std::string& channel_name);
		Channel* createChannel(const std::string& channel_name);
		void	removeChannel(const std::string& channel_name);
		void	quitFromAllChannels(int fd, const std::string &quit_msg);

		void	enablePollout(int fd);
		void	sendToClient(int fd, const std::string& message);
		void	broadcastToChannel(const std::string& channel_name, const std::string& message, int except_fd);
		void	broadcastToSharedUsers(int sender_fd, const std::string &message);

		Client* getClientByFd(int fd);
		Client* getClientByNickname(const std::string& nickname);
		
		const std::map<std::string, Channel> getChannels();
		const std::vector<pollfd> getfds();
		const std::string& getPassword() const;


};

#endif

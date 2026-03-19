#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <map>
#include <string>
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"
#include <vector>

class Server;

// IRC Reply Codes
const std::string RPL_WELCOME = "001";
const std::string ERR_NONICKNAMEGIVEN = "431";
const std::string ERR_ERRONEUSNICKNAME = "432";
const std::string ERR_NICKNAMEINUSE = "433";
const std::string ERR_NOTREGISTERED = "451";
const std::string ERR_NEEDMOREPARAMS = "461";
const std::string ERR_ALREADYREGISTRED = "462";
const std::string ERR_PASSWDMISMATCH = "464";
const std::string ERR_UNKNOWNCOMMAND = "421";

// Join Reply codes
const std::string RPL_NAMREPLY = "353";
const std::string RPL_ENDOFNAMES = "366";
const std::string ERR_NOSUCHCHANNEL = "403";
const std::string ERR_TOOMANYCHANNELS = "405";
const std::string ERR_BADCHANNELKEY = "475";
const std::string ERR_INVITEONLYCHAN = "473";
const std::string ERR_CHANNELISFULL = "471";

// privmsg
const std::string ERR_NOSUCHNICK = "401";
const std::string ERR_CANNOTSENDTOCHAN = "404";
const std::string ERR_NORECIPTIENT = "411";
const std::string ERR_NOTEXTTOSEND = "412";
// part
const std::string ERR_NOTONCHANNEL = "442";
// kick
const std::string ERR_USERNOTINCHANNEL = "441";
const std::string ERR_CHANOPRIVSNEEDED = "482";
// TOPIC
const std::string RPL_NOTOPIC = "331";
const std::string RPL_TOPIC = "332";
// invite
const std::string ERR_USERONCHANNEL = "443";
const std::string RPL_AWAY = "301";
const std::string RPL_INVITING = "341";
//mode
const std::string RPL_CHANNELMODEIS = "324";
const std::string ERR_UNKNOWNMODE = "472";
const std::string ERR_KEYSET = "467";
const std::string RPL_UMODEIS = "221";
const std::string ERR_UMODEUNKNOWNFLAG = "501";
const std::string ERR_USERSDONTMATCH = "502";
//ping
const std::string ERR_NOORIGIN = "409";
const std::string ERR_NOSUCHSERVER = "402";

class CommandHandler {
	private:
		Server& server;
		std::map<std::string, int>& nicknames;

		typedef void (CommandHandler::*CommandFunc)(Client&, const Message&);
		std::map<std::string, CommandFunc> commands;
		void	registerCommands();

		// params가 필요이상으로 들어오더라도, 필요한 만큼만 사용하고 나머지는 무시
		void	handlePass(Client& client, const Message& msg);
		void	handleNick(Client& client, const Message& msg);
		void	handleUser(Client& client, const Message& msg);
		void	handleJoin(Client& client, const Message& msg);
		void	handlePrivmsg(Client& client, const Message& msg);
		void	handlePart(Client& client, const Message& msg);
		void	handleKick(Client& client, const Message& msg);
		void	handleTopic(Client& client, const Message& msg);
		void	handleInvite(Client& client, const Message& msg);
		void	handleMode(Client& client, const Message& msg);
		void	handlePing(Client& client, const Message& msg);
		void	handleQuit(Client& client, const Message& msg);
		void	handleCap(Client& client, const Message& msg);
		void	applyMode(Client& client, const std::vector<std::string>& params, Channel* ch, const std::string &receiver);

		void	sendWelcome(Client& client);

	public:
		CommandHandler(std::map<std::string, int>& nicknames, Server& server);
		~CommandHandler();

		void handleCommand(Client& client, const Message& msg);
};

#endif

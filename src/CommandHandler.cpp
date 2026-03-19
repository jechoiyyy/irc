#include "CommandHandler.hpp"
#include "Server.hpp"
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <cstdlib>
#include <climits>
#include <errno.h>

static std::vector<std::string> split(const std::string &str, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, delimiter)) {
		if (!token.empty()) {
			tokens.push_back(token);
		}
	}
	return tokens;
}

CommandHandler::CommandHandler(std::map<std::string, int>& nicknames, Server& server):server(server), nicknames(nicknames)
{
	registerCommands();
}

CommandHandler::~CommandHandler() {}

void CommandHandler::sendWelcome(Client& client)
{
	std::string welcome = client.getNickname() + " :Welcome to Internet Relay Network " + client.getNickname() + "!" + client.getUsername() + "@localhost";
	client.send_reply(RPL_WELCOME, welcome);
}

void	CommandHandler::handlePass(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();
	if (client.is_pass_registered()) {
		client.send_reply(ERR_ALREADYREGISTRED, receiver + " :Unauthorized command (already registered)");
		return ;
	}

	if (msg.getParams().empty()) {
		client.send_reply(ERR_NEEDMOREPARAMS, receiver + " " + msg.getCommand() + " :Not enough parameters");
		return ;
	}

	std::string pass = msg.getParams()[0];
	if (pass == server.getPassword()) {
		client.setPassRegistered(true);
	} else {
		client.send_reply(ERR_PASSWDMISMATCH, receiver + " :Password incorrect");
	}
}

static bool	is_valid_nick(const std::string& nickname) 
{
	// nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
	// 첫 글자 이후 최대 8글자 더 올 수 있음. (최대길이 9)
	// 현대에는 보통 30자 내외로 설정
	// 첫 문자는 알파벳이나 special
	//		special =>  [, ], \, `, _, ^, {, |, }

	char c = nickname[0];
	if (isdigit(c) || c == '-' || c == '_')
		return false;
	if (!(nickname.size() > 1 && nickname.size() < 30))
		return false;
	return true;
}

void	CommandHandler::handleNick(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.is_pass_registered()) {
		client.send_reply(ERR_PASSWDMISMATCH, receiver + " :Password required");
		return ;
	}
	// 파라미터 체크
	if (msg.getParams().empty()) {
		client.send_reply(ERR_NONICKNAMEGIVEN, receiver + " :No nickname given");
		return ;
	}

	std::string new_nick = msg.getParams()[0];
	// 닉네임 유효성 체크
	if (!is_valid_nick(new_nick)) {
		client.send_reply(ERR_ERRONEUSNICKNAME, receiver + " :Erroneous nickname");
		return ;
	}

	// 중복 체크
	if (nicknames.find(new_nick) != nicknames.end()) {
		client.send_reply(ERR_NICKNAMEINUSE, receiver + " :Nickname is already in use");
		return ;
	}

	std::string old_nick = client.getNickname();
	bool was_registered = client.is_registered();

	// 이전 nickname 제거 (존재하면)
	if (!old_nick.empty()) {
		nicknames.erase(old_nick);
	}

	// 새로운 nickname 설정
	client.setNickname(new_nick);
	nicknames[new_nick] = client.getFd();

	if (was_registered) {
		std::string prefix = ":" + old_nick + "!" + client.getUsername() + "@localhost";
		std::string nick_msg = prefix + " NICK :" + new_nick + "\r\n";

		//본인 및 모든 유저에게 브로드캐스트
		server.sendToClient(client.getFd(), nick_msg);
		server.broadcastToSharedUsers(client.getFd(), nick_msg);
	} else if (client.is_registered()) {
		// 등록 된적 없고 방금 NICK 처리로 등록 완료 된 경우
		sendWelcome(client);
	}
}

void CommandHandler::handleUser(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.is_pass_registered()) {
		client.send_reply(ERR_PASSWDMISMATCH, receiver + " :Password required");
		return ;
	}
	// 이미 등록되어있는지
	if (client.is_registered()) {
		client.send_reply(ERR_ALREADYREGISTRED, receiver + " :Unauthorized command (already registered)");
		return ;
	}

	// 파라미터 개수 확인
	if (msg.getParams().size() < 4) {
		client.send_reply(ERR_NEEDMOREPARAMS, receiver + " " + msg.getCommand() + " :Not enough parameters");
		return ;
	}

	// username, realname 설정
	client.setUsername(msg.getParams()[0]);
	client.setRealname(msg.getParams()[3]);

	// 등록 확인 reply 전송
	if (client.is_registered())
		sendWelcome(client);
}

void CommandHandler::handleJoin(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.is_registered()) {
		client.send_reply(ERR_NOTREGISTERED, receiver + " :You have not registered");
		return ;
	}

	// 파라미터 확인
	// join :로 들어온 경우 빈 문자열로 파싱되므로 에러 조건 추가
	if (msg.getParams().empty() || msg.getParams()[0].empty()) {
		client.send_reply(ERR_NEEDMOREPARAMS, receiver + " JOIN :Not enough parameters");
		return ;
	}

	std::vector<std::string> channels = split(msg.getParams()[0], ',');
	std::vector<std::string> keys;
	if (msg.getParams().size() > 1) {
		keys = split(msg.getParams()[1], ',');
	}

	// 각 채널별 루프
	for (size_t i = 0; i < channels.size(); ++i) {
		std::string channel_name = channels[i];
		std::string key = (i < keys.size()) ? keys[i] : "";

		// 채널 이름 검증 (# 또는 &로 시작)
		if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&')) {
			client.send_reply(ERR_NOSUCHCHANNEL, receiver + " " + channel_name + " :No such channel");
			continue ;
		}

		// 채널 가져오기 또는 생성
		Channel* ch = server.getChannel(channel_name);
		if (!ch)
			ch = server.createChannel(channel_name);

		// 이미 멤버인지 확인 - 이미 멤버라면 무시 후 다음 채널
		if (ch->hasMember(client.getFd()))
			continue ;

		// 참가 가능 여부 확인 (mode 확인)
		if (!ch->canJoin(client.getFd(), key)) {
			if (ch->is_InviteOnly() && !ch->is_Invited(client.getFd()))
				client.send_reply(ERR_INVITEONLYCHAN, receiver + " " + channel_name + " :Cannot join channel (+i)");
			else if (ch->getUserLimit() > 0 && ch->getMembers().size() >= (size_t)ch->getUserLimit())
				client.send_reply(ERR_CHANNELISFULL, receiver + " " + channel_name + " :Cannot join channel (+l)");
			else if (!ch->getKey().empty() && ch->getKey() != key)
				client.send_reply(ERR_BADCHANNELKEY, receiver + " " + channel_name + " :Cannot join channel (+k)");
			continue ;
		}

		// 채널에 추가 (addMember 내부에서 Operator 처리)
		ch->addMember(client.getFd());
		ch->removeInvite(client.getFd());

		// JOIN 성공 메시지 전송: 본인(sendToClient), 다른 멤버(broadcastToChannel)
		std::string join_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost JOIN " + channel_name + "\r\n";
		server.sendToClient(client.getFd(), join_msg);
		server.broadcastToChannel(channel_name, join_msg, client.getFd());

		// Topic 응답
		if (ch->getChannelTopic().empty()) {
			client.send_reply(RPL_NOTOPIC, receiver + " " + channel_name + " :No topic is set");
		} else {
			client.send_reply(RPL_TOPIC, receiver + " " + channel_name + " :" + ch->getChannelTopic());
		}

		// Names 목록 전송
		std::string name_list;
		const std::set<int>& members = ch->getMembers();
		for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
			Client* member = server.getClientByFd(*it);
			if (member) {
				if (!name_list.empty()) {
					name_list += " ";
				}
				// 오퍼레이터면 @ 붙이기
				if (ch->isOperator(*it)) {
					name_list += "@";
				}
				name_list += member->getNickname();
			}
		}
		client.send_reply(RPL_NAMREPLY, receiver + " = " + channel_name + " :" + name_list);
		client.send_reply(RPL_ENDOFNAMES, receiver + " " + channel_name + " :End of /NAMES list");
	}
}

void	CommandHandler::handlePrivmsg(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.is_registered()) {
		client.send_reply(ERR_NOTREGISTERED, receiver + " :You have not registered");
		return ;
	}
	// 파라미터 확인
	if (msg.getParams().size() < 2 || msg.getParams()[1].empty()) {
		client.send_reply(ERR_NOTEXTTOSEND, receiver + " :No text to send");
		return ;
	}
	std::string target = msg.getParams()[0];
	std::string message = msg.getParams()[1];
	// 메시지 형식
	std::string full_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n"; 
	// 채널 메시지인가? (#또는 &로 시작)
	if (!target.empty() && (target[0] == '#' || target[0] == '&')) {
		Channel* ch = server.getChannel(target);

		if (!ch) {
			client.send_reply(ERR_NOSUCHCHANNEL, receiver + " " + target + " :No such channel");
			return ;
		}
		// 채널 멤버인지 확인
		if (!ch->hasMember(client.getFd())) {
			client.send_reply(ERR_CANNOTSENDTOCHAN, receiver + " " + target + " :Cannot send to channel");
			return ;
		}
		// 채널의 다른 멤버에게 브로드캐스트
		server.broadcastToChannel(target, full_msg, client.getFd());
	}
	// 개인메시지
	else {
		Client* rec = server.getClientByNickname(target);
		if (!rec) {
			client.send_reply(ERR_NOSUCHNICK, receiver + " " + target + " :No such nick/channel");
			return ;
		}
		server.sendToClient(rec->getFd(), full_msg);		
	}
}

// PART <channel> *("," <channel>) [ <Part Message> ]
void	CommandHandler::handlePart(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.is_registered()) {
		client.send_reply(ERR_NOTREGISTERED, receiver + " :You have not registered");
		return;
	}
	if (msg.getParams().empty() || msg.getParams()[0].empty()) {
		client.send_reply(ERR_NEEDMOREPARAMS, receiver + " PART :Not enough parameters");
		return ;
	}

	std::vector<std::string> channels = split(msg.getParams()[0], ',');

	std::string leave_msg;
	if (msg.getParams().size() > 1) {
		leave_msg = msg.getParams()[1];
	} else {
		leave_msg = client.getNickname();
	}

	for (size_t i = 0; i < channels.size(); ++i) {
		std::string channel_name = channels[i];
		Channel* ch = server.getChannel(channel_name);

		if (!ch) {
			client.send_reply(ERR_NOSUCHCHANNEL, receiver + " " + channel_name + " :No such channel");
			continue ;
		}
		if (!ch->hasMember(client.getFd())) {
			client.send_reply(ERR_NOTONCHANNEL, receiver + " " + channel_name + " :You're not on that channel");
			continue ;
		} 

		std::string part_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PART " + channel_name + " :" + leave_msg + "\r\n";

		server.sendToClient(client.getFd(), part_msg);
		server.broadcastToChannel(channel_name, part_msg, client.getFd());

		bool was_operator = ch->isOperator(client.getFd());
		ch->removeMember(client.getFd());

		if (ch->getMembers().empty()) {
			server.removeChannel(channel_name);
		} else {
			if (was_operator && ch->getOperators().empty()) {
				int new_op_fd = *(ch->getMembers().begin());
				ch->addOperator(new_op_fd);

				Client *new_op_client = server.getClientByFd(new_op_fd);
				if (new_op_client) {
					std::string mode_msg = ":localhost MODE " + channel_name + " +o " + new_op_client->getNickname() + "\r\n";
					server.broadcastToChannel(channel_name, mode_msg, -1);
				}
			}
		}
	}
}

// KICK <channel> *("," <channel> ) <user> *("," <user> ) [<comment>]
void	CommandHandler::handleKick(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.is_registered()) {
		client.send_reply(ERR_NOTREGISTERED, receiver + " :You have not registered");
		return;
	}
	if (msg.getParams().size() < 2) {
		client.send_reply(ERR_NEEDMOREPARAMS, receiver + " KICK :Not enough parameters");
		return ;
	}

	std::string channel_name = msg.getParams()[0];
	std::string target_nick = msg.getParams()[1];
	std::string reason;

	if (msg.getParams().size() > 2)
		reason = msg.getParams()[2];
	else
		reason = client.getNickname();
	// 채널 존재 확인
	Channel* ch = server.getChannel(channel_name);
	if (!ch) {
		client.send_reply(ERR_NOSUCHCHANNEL, receiver + " " + channel_name + " :No such channel");
		return ;
	}
	// 본인이 채널 멤버인지
	if (!ch->hasMember(client.getFd())) {
		client.send_reply(ERR_NOTONCHANNEL, receiver + " " + channel_name + " :You're not on that channel");
		return ;
	}
	// 본인이 오퍼레이터인지
	if (!ch->isOperator(client.getFd())) {
		client.send_reply(ERR_CHANOPRIVSNEEDED, receiver + " " + channel_name + " :You're not channel operator");
		return ;
	}
	// 대상 클라이언트 찾기
	Client* target = server.getClientByNickname(target_nick);
	if (!target) {
		client.send_reply(ERR_NOSUCHNICK, receiver + " " + target_nick + " :No such nick/channel");
		return ;
	}
	// 본인 kick 방지
	if (client.getFd() == target->getFd()) {
		std::string notice = ":localhost NOTICE " + channel_name + " " + client.getNickname() + " :You can't kick yourself for safety. \r\n";
		server.sendToClient(client.getFd(), notice);
		return ;
	}
	// 대상이 채널 멤버인지
	if (!ch->hasMember(target->getFd())) {
		client.send_reply(ERR_USERNOTINCHANNEL, receiver + " " + target_nick + " " + channel_name + " :They aren't on that channel");
		return ;
	}
	// KICK 메시지 구성
	std::string kick_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + channel_name + " " + target_nick + " :" + reason + "\r\n";
	// 브로드캐스트
	server.sendToClient(target->getFd(), kick_msg);
	server.broadcastToChannel(channel_name, kick_msg, target->getFd());
	// 대상을 채널에서 제거
	
	ch->removeMember(target->getFd());
	// 채널 비어있으면 채널 제거
	if (ch->getMembers().empty()) {
		server.removeChannel(channel_name);
	} else {
		if (ch->getOperators().empty()) {
			int new_op_fd = *(ch->getMembers().begin());
			ch->addOperator(new_op_fd);

			Client *new_op = server.getClientByFd(new_op_fd);
			if (new_op) {
				std::string mode_msg = ":localhost MODE " + channel_name + " +o " + new_op->getNickname() + "\r\n";
				server.broadcastToChannel(channel_name, mode_msg, -1);
			}
		}
	}
}

// TOPIC <channel> [ <topic> ]
// - 매개변수가 없으면, 해당 채널의 토픽 반환
//   - 조회는 관리자가 아니여도 가능
// - 매개변수가 “:”이면, 해당 채널의 토픽 삭제
// - 채널 뒤에 매개변수가 있으면, 채널이 허용하는 경우에 토픽을 변경
//   - ":another topic" => "another topic"으로 set
void	CommandHandler::handleTopic(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.is_registered()) {
		client.send_reply(ERR_NOTREGISTERED, receiver + " :You have not registered");
		return;
	}

	if (msg.getParams().empty() || msg.getParams()[0].empty()) {
		client.send_reply(ERR_NEEDMOREPARAMS, receiver + " TOPIC :Not enough parameters");
		return ;
	}

	std::string channel_name = msg.getParams()[0];

	Channel* ch = server.getChannel(channel_name);
	if (!ch) {
		client.send_reply(ERR_NOSUCHCHANNEL, receiver + " " + channel_name + " :No such channel");
		return ;
	}
	
	if (!ch->hasMember(client.getFd())) {
		client.send_reply(ERR_NOTONCHANNEL, receiver + " " + channel_name + " :You're not on that channel");
		return ;
	}

	if (msg.getParams().size() == 1) {
		std::string topic = ch->getChannelTopic();
		if (!topic.empty()) {
			client.send_reply(RPL_TOPIC, receiver + " " + channel_name + " :" + ch->getChannelTopic());
		} else {
			client.send_reply(RPL_NOTOPIC, receiver + " " + channel_name + " :No topic is set");
		}
		return ;
	}
	
	std::string change_topic = msg.getParams()[1];
	if (ch->is_TopicRestricted()) {
		// +t모드: 관리자만 변경가능
		if (!ch->isOperator(client.getFd())) {
			client.send_reply(ERR_CHANOPRIVSNEEDED, receiver + " " + channel_name + " :You're not channel operator");
			return ;
		}
	}
	ch->setTopic(change_topic);
	std::string topic_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost TOPIC " + channel_name + " :" + change_topic + "\r\n";

	server.sendToClient(client.getFd(), topic_msg);
	server.broadcastToChannel(channel_name, topic_msg, client.getFd());
}

// INVITE <nickname> <channel>
// - +i mode일 때는 운영자만 초대할 수 있음.
// - 존재하는 채널에 초대하기 위한 명령어
//	- 존재하지 않는 채널에 초대하면, no such channel
//	- 존재하는 채널이라면, 채널의 구성원만 초대 가능
//	  - +i mode일 때는, 운영자만 초대 가능
//	- 초대하는 유저와 초대받는 유저만 초대에 대한 reply 받음
void	CommandHandler::handleInvite(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.is_registered()) {
		client.send_reply(ERR_NOTREGISTERED, receiver + " :You have not registered");
		return;
	}

	if (msg.getParams().size() < 2) {
		client.send_reply(ERR_NEEDMOREPARAMS, receiver + " INVITE :Not enough parameters");
		return ;
	}
	std::string target_nick = msg.getParams()[0];
	std::string channel_name = msg.getParams()[1];

	Client* target = server.getClientByNickname(target_nick);
	if (!target) {
		client.send_reply(ERR_NOSUCHNICK, receiver + " " + target_nick + " :No such nick/channel");
		return ;
	}

	Channel* ch = server.getChannel(channel_name);
	if (!ch) {
		client.send_reply(ERR_NOSUCHCHANNEL, receiver + " " + channel_name + " :No such channel");
		return ;
	}

	if (!ch->hasMember(client.getFd())) {
		client.send_reply(ERR_NOTONCHANNEL, receiver + " " + channel_name + " :You're not on that channel");
		return ;
	}

	if (ch->is_InviteOnly()) {
		if (!ch->isOperator(client.getFd())) {
			client.send_reply(ERR_CHANOPRIVSNEEDED, receiver + " " + channel_name + " :You're not channel operator");
			return ;
		}
	}

	if (ch->hasMember(target->getFd())) {
		client.send_reply(ERR_USERONCHANNEL, receiver + " " + target_nick + " " + channel_name + " :is already on channel");
		return ;
	}
	ch->addInvite(target->getFd());
	client.send_reply(RPL_INVITING, receiver + " " + channel_name + " " + target_nick);
	std::string invite_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost INVITE " + target_nick + " " + channel_name + "\r\n";
	server.sendToClient(target->getFd(), invite_msg);
}

/* 
MODE <channel> *( ("-" / "+" ) *<modes> *<modeparams> )
- i : 초대 전용 채널
	- -i: 운영자, 일반유저 둘 다 초대 가능
	- +i: 운영자만 초대 가능
- t : 채널 운영자만 설정 할 수 있는 토픽 플래그
	- -t: 모두가 토픽 변경 가능
	- +t: 운영자만 토픽 변경 가능
- k : 채널 비밀번호 설정 (운영자만 설정가능)
	- MODE #42 -k oulu  => #42채널에서 oulu키 삭제
	- MODE #42 +k oulu  => #42채널에서 oulu키 생성
		- 채널키는 최대 23자 (공백이나 제어문자 포함 안됨)
		- 이미 key가 있다면, -k로 제거를 먼저 해야 함.
			- 무시하거나 err처리
- o : 채널 운영자 권한 부여 / 박탈
	- -o: 운영자 권한 박탈
	- +o: 운영자 권한 부여
- l : set the user limit to channel (운영자만 설정 가능)
	- -l: user limit해제
	- +l <limit> : user limit 설정
채널 모드 조회는 해당 클라이언트한테만
채널 모드 변경사항은 브로드캐스트(호출자도 포함)
*/
// 예시: MODE #42 +kl jechoi 10   (동시 변경 가능)
void	CommandHandler::handleMode(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.is_registered()) {
		client.send_reply(ERR_NOTREGISTERED, receiver + " :You have not registered");
		return;
	}
	if (msg.getParams().size() < 1) {
		client.send_reply(ERR_NEEDMOREPARAMS, receiver + " MODE :Not enough parameters");
		return ;
	}

	std::string target = msg.getParams()[0];
	
	// 유저 모드 처리
	if (target[0] != '#' && target[0] != '&') {
		if (target == client.getNickname()) {
			if (msg.getParams().size() == 1) {
				// 본인 모드 조회 요청 시 모드 없음 응답
				client.send_reply(RPL_UMODEIS, receiver + " +");
				return ;
			}
			// irssi 접속 시 MODE <nickname> +i 요청 응답 무시
			return ;
		}
		client.send_reply(ERR_USERSDONTMATCH, receiver + " :Cannot change mode for other users");
		return ;
	}

	Channel* ch = server.getChannel(target);
	if (!ch) {
		client.send_reply(ERR_NOSUCHCHANNEL, receiver + " " + target + " :No such channel");
		return ;
	}
	if (msg.getParams().size() == 1) {
		// 현재 모드 출력
		client.send_reply(RPL_CHANNELMODEIS, receiver + " " + target + " " + ch->getMode());
		return ;
	}
	if (!ch->hasMember(client.getFd())) {
		client.send_reply(ERR_NOTONCHANNEL, receiver + " " + target + " :You're not on that channel");
		return ;
	}

	if (!ch->isOperator(client.getFd())) {
		client.send_reply(ERR_CHANOPRIVSNEEDED, receiver + " " + target + " :You're not channel operator");
		return ;
	}
	applyMode(client, msg.getParams(), ch, receiver);
}

// i t k o l
// ERR_UNKNOWNMODE(472) : 해당 모드를 지원하지 않음.
// i
// t
// k
// 	ERR_NEEDMOREPARAMS : key가 없을 때
// 	ERR_KEYSET(467) : 이미 키가 있음
// o
// 	ERR_NEEDMOREPARAMS : 대상 닉네임이 없을 때
// 	ERR_USERNOTINCHANNEL: 임명할려는데, 그 target이 채널에 없을 때
// l
// 	ERR_NEEDMOREPARAMS : 숫자 인자가 없을 때
void	CommandHandler::applyMode(Client& client, const std::vector<std::string>& params, Channel* ch, const std::string &receiver)
{
	std::string	modes = params[1];
	size_t params_index = 2;

	bool	mode_on = true;
	char	sign_cur = '+';
	bool	sign_changed = false;

	std::string result_modes = "";
	std::string result_params = "";

	for (size_t i = 0; i < modes.size(); ++i) {
		char c = modes[i];

		if (c == '+') {
			mode_on = true;
			sign_cur = '+';
			sign_changed = true;
			continue;
		}
		if (c == '-') {
			mode_on = false;
			sign_cur = '-';
			sign_changed = true;
			continue;
		}

		bool mode_applied = false;

		if (c == 'i') {
			if (mode_on) {
				if (!ch->is_InviteOnly()) {
					ch->setInviteOnly(true);
					mode_applied = true;
				}
			} else {
				if (ch->is_InviteOnly()) {
					ch->setInviteOnly(false);
					mode_applied = true;
				}
			}
		} else if (c == 't') {
			
			if (mode_on) {
				if (!ch->is_TopicRestricted()) {
					ch->setTopicRestricted(true);
					mode_applied = true;
				}
			} else {
				if (ch->is_TopicRestricted()) {
					ch->setTopicRestricted(false);
					mode_applied = true;
				}
			}
		} else if (c == 'k') {
			if (mode_on) {
				if (params_index < params.size()) {
					std::string key = params[params_index++];
					if (!ch->getKey().empty()) {
						client.send_reply(ERR_KEYSET, receiver + " " + ch->getChannelName() + " :Channel key already set");
					} else {
						ch->setKey(key);
						mode_applied = true;
						result_params += key + " ";
					}
				} else {
					client.send_reply(ERR_NEEDMOREPARAMS, receiver + " MODE +k :Not enough parameters");
				}
			} else {
				if (params_index < params.size()) {
					std::string key = params[params_index++];;
					if (!ch->getKey().empty() && ch->getKey() == key) {
						ch->setKey("");
						mode_applied = true;
						result_params += key + " ";
					}
				}
			}
		} else if (c == 'o') {
			if (params_index < params.size()) {
				std::string nick = params[params_index];
				++params_index;

				Client* target = server.getClientByNickname(nick);
				if (!target) {
					client.send_reply(ERR_NOSUCHNICK, receiver + " " + nick + " :No such nick");
					continue;
				}

				if (!ch->hasMember(target->getFd())) {
					client.send_reply(ERR_USERNOTINCHANNEL, receiver + " " + nick + " " + ch->getChannelName() + " :They aren't on that channel");
					continue;
				}

				if (mode_on) {	// +o (권한 부여)
					if (!ch->isOperator(target->getFd())) {
						ch->addOperator(target->getFd());
						mode_applied = true;
						result_params += nick + " ";
					}
				} else {		// -o (권한 박탈)
					if (client.getFd() == target->getFd()) {
						continue;
					}
					if (ch->isOperator(target->getFd())) {
						ch->removeOperator(target->getFd());
						mode_applied = true;
						result_params += nick + " ";
					}
				}
			} else {
				client.send_reply(ERR_NEEDMOREPARAMS, receiver + " MODE " + (mode_on ? "+o" : "-o") + " :Not enough parameters");
				continue;
			}
		} else if (c == 'l') {
			if (mode_on) {
				if (params_index < params.size()) {
					std::string limit_str = params[params_index++];

					bool valid = true;
					for (size_t i = 0; i < limit_str.length(); i++) {
						if (!isdigit(limit_str[i])) {
							valid = false;
							break;
						}
					}
					if (valid && !limit_str.empty()) {
						errno = 0;
						long val = std::strtol(limit_str.c_str(), NULL, 10);
						int limit;
						if (errno == ERANGE || val > INT_MAX) {
							limit = INT_MAX;
						} else {
							limit = static_cast<int>(val);
						}

						if (limit > 0 && limit != ch->getUserLimit()) {
							ch->setUserLimit(limit);
							mode_applied = true;
							result_params += limit_str + " ";
						}
					}
				} else {
					client.send_reply(ERR_NEEDMOREPARAMS, receiver + " MODE +l :Not enough parameters");
				}
			} else {
				if (ch->getUserLimit() > 0) {
					ch->setUserLimit(0);
					mode_applied = true;
				}
			}
		} else {
			std::string unknown_msg;
			unknown_msg += c;
			client.send_reply(ERR_UNKNOWNMODE, receiver + " " + unknown_msg + " :is unknown mode char to me for " + ch->getChannelName());
		}

		// mode 성공 적용 시에만 부호와 모드 문자 추가
		if (mode_applied) {
			if (sign_changed) {
				result_modes += sign_cur;
				sign_changed = false;
			}
			result_modes += c;
		}
	}

	// 브로드 캐스트
	if (!result_modes.empty()) {
		std::string mode_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost MODE " + ch->getChannelName() + " " + result_modes;

		if (!result_params.empty()) {
			result_params.resize(result_params.size() - 1);
			mode_msg += " " + result_params;
		}
		mode_msg += "\r\n";

		server.sendToClient(client.getFd(), mode_msg);
		server.broadcastToChannel(ch->getChannelName(), mode_msg, client.getFd());
	}
}

void	CommandHandler::handlePing(Client& client, const Message& msg)
{
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();

	// 파라미터가 하나도 없는 경우
	if (msg.getParams().empty()) {
		client.send_reply(ERR_NOORIGIN, receiver + " :No origin specified");
		return ;
	}

	// PONG 응답 전송
	// PING에서 받는 토큰 그대로 포함
	std::string token = msg.getParams()[0];

	// 형식:    :<servername> PONG <servername> :<token>
	std::string pong_reply = ":localhost PONG localhost :" + token + "\r\n";
	server.sendToClient(client.getFd(), pong_reply);
}

// QUIT :nick
// QUIT :[msg]
void	CommandHandler::handleQuit(Client& client, const Message& msg)
{
	std::string reason = "Client Quit";
	if (!msg.getParams().empty()) {
		reason = msg.getParams()[0];
	}

	client.setDisconnecting(reason);

	std::string error_msg = "ERROR :Closing Link: localhost (" + reason + ")\r\n";
	server.sendToClient(client.getFd(), error_msg);
}

void	CommandHandler::handleCap(Client& client, const Message& msg)
{
	if (msg.getParams().empty())
		return ;

	std::string subCommand = msg.getParams()[0];

	if (subCommand == "LS") {
		std::string reply = ":localhost CAP * LS :\r\n";
		server.sendToClient(client.getFd(), reply);
	}
}

void	CommandHandler::registerCommands()
{
	commands["PASS"] = &CommandHandler::handlePass;
	commands["NICK"] = &CommandHandler::handleNick;
	commands["USER"] = &CommandHandler::handleUser;
	commands["JOIN"] = &CommandHandler::handleJoin;
	commands["PRIVMSG"] = &CommandHandler::handlePrivmsg;
	commands["PART"] = &CommandHandler::handlePart;
	commands["KICK"] = &CommandHandler::handleKick;
	commands["TOPIC"] = &CommandHandler::handleTopic;
	commands["INVITE"] = &CommandHandler::handleInvite;
	commands["MODE"] = &CommandHandler::handleMode;
	commands["PING"] = &CommandHandler::handlePing;
	commands["QUIT"] = &CommandHandler::handleQuit;
	commands["CAP"] = &CommandHandler::handleCap;
}

void CommandHandler::handleCommand(Client& client, const Message& msg)
{
	if (msg.getCommand().empty())
		return ;

	std::string cmd = msg.getCommand();
	std::string receiver = client.getNickname().empty() ? "*" : client.getNickname();
	std::map<std::string, CommandFunc>::iterator it = commands.find(cmd);

	if (it != commands.end()) {
		CommandFunc func = it->second;
		return (this->*func)(client, msg);
	}

	client.send_reply(ERR_UNKNOWNCOMMAND, receiver + " " + cmd + " :Unknown command");
	std::cout << "Unknown command: " << cmd << std::endl;
	return ;
}

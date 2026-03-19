#include "Channel.hpp"
#include <sstream>
#include <string>

Channel::Channel(): channel_name_(""), invite_only(false), topic_restricted(true), user_limit(0) {}

Channel::Channel(const std::string& name): channel_name_(name), invite_only(false), topic_restricted(true), user_limit(0) {}

void	Channel::addMember(int client_fd)
{
	channel_members_.insert(client_fd);

	if (channel_members_.size() == 1)
		channel_operators_.insert(client_fd);
}

void	Channel::removeMember(int client_fd)
{
	channel_members_.erase(client_fd);
	removeOperator(client_fd);
	invite_list.erase(client_fd);
}

bool	Channel::hasMember(int client_fd) const
{
	return channel_members_.find(client_fd) != channel_members_.end();
}

void	Channel::addOperator(int client_fd)
{
	if (hasMember(client_fd)) {
		channel_operators_.insert(client_fd);
	}
}

void	Channel::removeOperator(int client_fd)
{
	channel_operators_.erase(client_fd);
}

bool	Channel::isOperator(int client_fd) const
{
	return channel_operators_.find(client_fd) != channel_operators_.end();
}

void	Channel::addInvite(int client_fd) 
{
	invite_list.insert(client_fd);
}

void	Channel::removeInvite(int client_fd)
{
	invite_list.erase(client_fd);
}

bool	Channel::is_Invited(int client_fd) const
{
	return invite_list.find(client_fd) != invite_list.end();
}

const std::set<int>&	Channel::getMembers() const
{
	return channel_members_;
}

const std::set<int>&	Channel::getOperators() const
{
	return channel_operators_;
}

const std::string&	Channel::getChannelName() const
{
	return channel_name_;
}

const std::string&	Channel::getChannelTopic() const
{
	return channel_topic_;
}

const std::string&	Channel::getKey() const
{
	return key;
}

int	Channel::getUserLimit() const
{
	return user_limit;
}

// :server 324 <nick> <channel> <modes> <mode_params>
// :localhost 324 alice #42 +ntkl oulu 50
//	i t k l
const std::string	Channel::getMode() const
{
	std::string modes = "+";
	std::string params = "";
	if (invite_only)
		modes += "i";
	if (topic_restricted)
		modes += "t";
	if (!key.empty()) {
		modes += "k";
		params = key;
	}
	if (user_limit > 0) {
		modes += "l";
		std::ostringstream os;
		os << user_limit;
		// 파라미터 이미 존재한다면 공백으로 구분
		if (!params.empty())
			params += " ";
		params += os.str();
	}
	std::string result = params.empty() ? modes : modes + " " + params;
	return result;
}

void	Channel::setTopic(const std::string& topic)
{
	channel_topic_ = topic;
}

void	Channel::setKey(const std::string& new_key)
{
	key = new_key;
}

void	Channel::setUserLimit(int limit)
{
	user_limit = limit;
}

bool	Channel::is_InviteOnly() const
{
	return invite_only;
}

bool	Channel::is_TopicRestricted() const
{
	return topic_restricted;
}

void	Channel::setInviteOnly(bool value)
{
	invite_only = value;
}

void	Channel::setTopicRestricted(bool value)
{
	topic_restricted = value;
}

bool	Channel::canJoin(int client_fd, const std::string& provided_key) const
{
	if (hasMember(client_fd))
		return true;

	if (is_Invited(client_fd))
		return true;

	if (invite_only)
		return false;

	if (user_limit > 0 && channel_members_.size() >= (size_t)user_limit)
		return false;
	if (!key.empty() && provided_key != key)
		return false;
	return true;
}

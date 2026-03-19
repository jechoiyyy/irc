#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <set>

class Channel {
	private:
		std::string channel_name_;
		std::string	channel_topic_;
		std::set<int> channel_operators_;
		std::set<int> channel_members_;

		bool	invite_only;
		bool	topic_restricted;
		std::string	key;
		int user_limit;
		std::set<int> invite_list;

	public:
		Channel();
		Channel(const std::string& name);

		void	addMember(int client_fd);
		void	removeMember(int client_fd);
		bool	hasMember(int client_fd) const;

		void	addOperator(int client_fd);
		void	removeOperator(int client_fd);
		bool	isOperator(int client_fd) const;

		void	addInvite(int client_fd);
		void	removeInvite(int client_fd);
		bool	is_Invited(int client_fd) const;

		const std::set<int>&	getMembers() const;
		const std::set<int>&	getOperators() const;
		const std::string&	getChannelName() const;
		const std::string&	getChannelTopic() const;
		const std::string&	getKey() const;
		int	getUserLimit() const;
		const std::string	getMode() const;

		bool	is_InviteOnly() const;
		bool	is_TopicRestricted() const;

		void	setInviteOnly(bool value);
		void	setTopicRestricted(bool value);
		void	setTopic(const std::string& topic);
		void	setKey(const std::string& key);
		void	setUserLimit(int limit);

		bool	canJoin(int client_fd, const std::string& provided_key) const;
};

#endif
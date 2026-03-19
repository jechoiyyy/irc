#include "Message.hpp"
#include <iostream>

Message::Message() {}

Message::Message(const std::string& raw) {

	std::string msg = raw;
	size_t pos;

	if (msg.empty())
		return ;
	
	if (msg[0] == ':') {
		pos = msg.find(' ');
		if (pos == std::string::npos) {
			std::cerr << "Invalid message format: " << raw << std::endl;
			return ;
		}
		prefix = msg.substr(1, pos - 1);
		msg = msg.substr(pos + 1);
		trimLeadingSpace(msg);
	}

	pos = msg.find(' ');
	if (pos == std::string::npos) {
		command = msg;
		toUpperCase(this->command);
		return ;
	}

	command = msg.substr(0, pos);
	toUpperCase(this->command);
	msg = msg.substr(pos + 1);
	trimLeadingSpace(msg);

	while (!msg.empty()) {
		trimLeadingSpace(msg);
		if (msg.empty())
			break;

		if (msg[0] == ':') {
			params.push_back(msg.substr(1));
			break ;
		}

		pos = msg.find(' ');
		if (pos == std::string::npos) {
			params.push_back(msg);
			break;
		}

		if (params.size() < 14) {
			params.push_back(msg.substr(0, pos));
			msg = msg.substr(pos + 1);
		} else {
			params.push_back(msg.substr(0, msg.size()));
			msg = "";
		}
	}
}

void	Message::toUpperCase(std::string& str) {
	for (size_t i = 0; i < str.size(); ++i) {
		if (str[i] >= 'a' && str[i] <= 'z')
			str[i] -= 32;
	}
}

void	Message::trimLeadingSpace(std::string& str) {
	while (!str.empty() && str[0] == ' ')
		str = str.substr(1);
}

const std::string& Message::getPrefix() const {
	return prefix;
}

const std::string& Message::getCommand() const {
	return command;
}

const std::vector<std::string>& Message::getParams() const {
	return params;
}

bool	Message::is_Empty() const {
	return command.empty();
}

#ifndef MESSAGE_HPP
# define MESSAGE_HPP

#include <vector>
#include <string>
#include <iostream>

class Message {
	private:
		std::string prefix;
		std::string command;
		std::vector<std::string> params; // 매개변수는 최대 15개를 가질 수 있음.
			// params가 15개일 때, 마지막 인자는 ':'가 있던없든 무조건 tailing으로 간주함. (공백 포함 가능)

		void	toUpperCase(std::string& str);
		void	trimLeadingSpace(std::string& str);

	public:
		Message();
		explicit Message(const std::string& raw);

		// getter
		const std::string& getPrefix() const;
		const std::string& getCommand() const;
		const std::vector<std::string>& getParams() const;

		bool is_Empty() const;
};

#endif

# ft_irc

_This project has been created as part of the 42 curriculum by jechoi, seokson_

## Description

ft_irc is a C++98 implementation of an IRC (Internet Relay Chat) server. The project aims to create a fully functional IRC server that handles multiple clients simultaneously using non-blocking I/O operations. The server supports essential IRC commands, channel management, and private messaging, allowing users to connect using standard IRC clients.

## Instructions

### Compilation

```bash
make
```

### Running the Server

```bash
./ircserv <port> <password>
```

**Parameters:**

- `port`: The port number on which the server will listen for incoming connections
- `password`: The connection password required for clients to connect to the server

**Example:**

```bash
./ircserv 6667 mypassword
```

### Checking Whether the Server is listening on IPv4 or IPv6

```bash
netstat -an | grep 6667
```

### Connecting to the Server

Use `irssi` to connect to the server:

```bash
irssi -c localhost -p 6667 -w 1234 -n <nickname>
```

After connecting, you can join a channel and start chatting:

```text
/join #general
hello everyone
```

### Clean up

```bash
make clean   # Remove object files
make fclean  # Remove object files and executable
make re      # Rebuild the project
```

### Mandatory Part commands

`PASS`: Authenticates the client with the server password before registration.

```text
PASS mypassword
```

`NICK`: Sets or changes the client's nickname.

```text
NICK jechoi
```

`USER`: Completes user registration by sending username and real name information.
USER <username> <hostname> <servername> :<realname>

```text
USER jechoi 0 * :Jechoi
```

`JOIN`: Joins an existing channel or creates it if it does not exist.

```text
JOIN #general
```

`PRIVMSG`: Sends a private message to a user or a channel.

```text
PRIVMSG #general :Hello everyone!
```

`KICK`: Removes a user from a channel. Usually available to channel operators only.

```text
KICK #general troublemaker :Spamming
```

`INVITE`: Invites a user to join an invite-only channel.

```text
INVITE alice #general
```

`TOPIC`: Sets or displays the topic of a channel.

```text
TOPIC #general :Welcome to the general room
```

`MODE`: Changes channel modes such as invite-only, topic protection, key, operator privilege, and user limit.

```text
MODE #general +i
```

#### Supported channel modes

- `MODE #general +i`: Enable invite-only mode
- `MODE #general -i`: Disable invite-only mode

- `MODE #general +t`: Only channel operators can change the topic
- `MODE #general -t`: Allow all channel members to change the topic

- `MODE #general +k secret`: Set a channel password
- `MODE #general -k secret`: Remove the channel password

- `MODE #general +o alice`: Grant operator privileges to `alice`
- `MODE #general -o alice`: Remove operator privileges from `alice`

- `MODE #general +l 10`: Limit the channel to 10 users
- `MODE #general -l`: Remove the user limit

### Bonus

#### File Transfer

In IRC, file transfer is handled through the DCC (Direct Client-to-Client) protocol.
The server does not relay the file data itself; it only acts as a mediator to establish the transfer between clients.

Send file aa to bb

```text
/dcc send bb fileA
```

Get file bb from aa

```text
/dcc get aa
```

#### Bot

An IRC bot is implemented as an external client that complies with the RFC 1459/2812 protocols.
It runs independently from the server process and interacts with the server through a socket connection.

Use 'ircbot' to connect to the server:

```bash
./ircbot localhost 6667 1234
```

Inviting 'ircbot' to a channel:

```text
/invite ircbot <channel>
```

What a bot can do:

- !help: Displays a list of commands the bot can perform.
- !hello / !hi: The bot greets you.
- !ping: The bot responds with “pong.”
- !dice: The bot rolls a dice.
- !time: The bot shows the current server time.
- !42: 42!

## Resources

**IRC Protocol Documentation:**

- [RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459) - Internet Relay Chat Protocol
- [RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812) - Internet Relay Chat: Client Protocol

**Technical References:**

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

**AI Usage:**
AI tools were used for:

- Debugging and error resolution during development
- Understanding IRC protocol specifications and edge cases
- Code review and optimization suggestions for the command handler and message parsing logic
- Translating sentences that are difficult to articulate

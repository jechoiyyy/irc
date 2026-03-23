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

### Bonus
#### File Transfer

IRC에서 file transfer는 dcc(direct client to client) 프로토콜을 통해 이루어진다.
서버는 파일 데이터의 통로가 아니라 클라이언트 간 전송을 성립시켜주는 중개의 역할만 한다.

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

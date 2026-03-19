_This project has been created as part of the 42 curriculum by jechoi_

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
irssi
```

Then connect from the `irssi` prompt:

```text
/connect localhost 6667 mypassword
```

If you want to use a specific nickname when connecting:

```text
/connect -nick mynick localhost 6667 mypassword
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

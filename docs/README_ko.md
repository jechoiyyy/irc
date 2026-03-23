_이 프로젝트는 jechoi와 seokson이 42 커리큘럼의 일환으로 작성했습니다_

## 설명

ft_irc는 C++98로 구현한 IRC(Internet Relay Chat) 서버입니다. 이 프로젝트의 목표는 논블로킹 I/O를 사용해 여러 클라이언트를 동시에 처리할 수 있는 완전한 IRC 서버를 만드는 것입니다. 서버는 필수 IRC 명령어, 채널 관리, 개인 메시지 전송을 지원하며, 사용자는 표준 IRC 클라이언트로 서버에 접속할 수 있습니다.

## 사용 방법

### 컴파일

```bash
make
```

### 서버 실행

```bash
./ircserv <port> <password>
```

**매개변수:**

- `port`: 서버가 클라이언트 연결을 수신할 포트 번호
- `password`: 클라이언트가 서버에 접속할 때 필요한 비밀번호

**예시:**

```bash
./ircserv 6667 mypassword
```

### 서버 접속

`irssi`를 사용해 서버에 접속할 수 있습니다:

```bash
irssi -c localhost -p 6667 -w 1234 -n <nickname>
```

접속한 뒤에는 채널에 입장해 대화를 시작할 수 있습니다:

```text
/join #general
hello everyone
```

### 정리

```bash
make clean   # 오브젝트 파일 삭제
make fclean  # 오브젝트 파일과 실행 파일 삭제
make re      # 프로젝트 다시 빌드
```

### Mandatory Part 명령어

`PASS`: 등록 전에 서버 비밀번호로 클라이언트를 인증합니다.

```text
PASS mypassword
```

`NICK`: 클라이언트의 닉네임을 설정하거나 변경합니다.

```text
NICK jechoi
```

`USER`: 사용자 이름과 실제 이름 정보를 전송하여 사용자 등록을 완료합니다.
USER <username> <hostname> <servername> :<realname>

```text
USER jechoi 0  * :Jechoi
```

`JOIN`: 기존 채널에 입장하거나, 채널이 없으면 새로 생성합니다.

```text
JOIN #general
```

`PRIVMSG`: 사용자 또는 채널에 개인 메시지를 전송합니다.

```text
PRIVMSG #general :Hello everyone!
```

`KICK`: 채널에서 사용자를 강퇴합니다. 일반적으로 채널 운영자만 사용할 수 있습니다.

```text
KICK #general troublemaker :Spamming
```

`INVITE`: 사용자를 초대 전용 채널에 초대합니다.

```text
INVITE alice #general
```

`TOPIC`: 채널의 주제를 조회하거나 설정합니다.

```text
TOPIC #general :Welcome to the general room
```

`MODE`: 초대 전용, 주제 변경 제한, 비밀번호, 운영자 권한, 인원 제한과 같은 채널 모드를 변경합니다.

```text
MODE #general +i
```

#### 지원하는 채널 모드

- `MODE #general +i`: 초대 전용 모드를 활성화합니다
- `MODE #general -i`: 초대 전용 모드를 비활성화합니다

- `MODE #general +t`: 채널 운영자만 주제를 변경할 수 있도록 설정합니다
- `MODE #general -t`: 모든 채널 멤버가 주제를 변경할 수 있도록 설정합니다

- `MODE #general +k secret`: 채널 비밀번호를 설정합니다
- `MODE #general -k secret`: 채널 비밀번호를 제거합니다

- `MODE #general +o alice`: `alice`에게 운영자 권한을 부여합니다
- `MODE #general -o alice`: `alice`의 운영자 권한을 제거합니다

- `MODE #general +l 10`: 채널 인원을 10명으로 제한합니다
- `MODE #general -l`: 인원 제한을 해제합니다

### Bonus

#### 파일 전송

IRC에서 파일 전송은 DCC(Direct Client-to-Client) 프로토콜을 통해 이루어집니다.
서버는 파일 데이터를 직접 중계하지 않고, 클라이언트 간 전송이 이루어지도록 연결을 성립시키는 역할만 합니다.

aa가 bb에게 파일 보내기

```text
/dcc send bb fileA
```

aa가 bb에게서 파일 받기

```text
/dcc get aa
```

#### 봇

IRC 봇은 RFC 1459/2812 프로토콜을 준수하는 외부 클라이언트로 구현되어 있습니다.
봇은 서버 프로세스와 독립적으로 실행되며, 소켓 연결을 통해 서버와 상호작용합니다.

`ircbot`으로 서버에 접속하기:

```bash
./ircbot localhost 6667 1234
```

채널에 `ircbot` 초대하기:

```text
/invite ircbot <channel>
```

봇이 할 수 있는 기능:

- `!help`: 봇이 수행할 수 있는 명령어 목록을 보여줍니다.
- `!hello` / `!hi`: 봇이 인사합니다.
- `!ping`: 봇이 `pong`으로 응답합니다.
- `!dice`: 봇이 주사위를 굴립니다.
- `!time`: 봇이 현재 서버 시간을 보여줍니다.
- `!42`: 42!

## 참고 자료

**IRC 프로토콜 문서:**

- [RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459) - Internet Relay Chat Protocol
- [RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812) - Internet Relay Chat: Client Protocol

**기술 참고 자료:**

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

**AI 사용:**
AI 도구는 다음과 같은 용도로 사용되었습니다:

- 개발 중 디버깅 및 오류 해결
- IRC 프로토콜 사양과 예외 상황 이해
- 명령어 핸들러와 메시지 파싱 로직에 대한 코드 리뷰 및 최적화 제안
- 자연스럽게 표현하기 어려운 문장의 번역

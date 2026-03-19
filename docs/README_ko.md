_이 프로젝트는 jechoi에 의해 42 커리큘럼의 일환으로 작성되었습니다_

## 설명

ft_irc는 C++98로 구현한 IRC(Internet Relay Chat) 서버입니다. 이 프로젝트는 논블로킹 I/O 작업을 사용하여 여러 클라이언트를 동시에 처리할 수 있는 완전한 기능을 갖춘 IRC 서버를 만드는 것을 목표로 합니다. 서버는 필수 IRC 명령어, 채널 관리, 개인 메시지 전송을 지원하며, 사용자가 표준 IRC 클라이언트를 사용하여 연결할 수 있습니다.

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

- `port`: 서버가 들어오는 연결을 수신할 포트 번호
- `password`: 클라이언트가 서버에 연결하는 데 필요한 접속 비밀번호

**예시:**

```bash
./ircserv 6667 mypassword
```

### 서버 연결

`irssi`를 사용하여 서버에 접속할 수 있습니다:

```bash
irssi
```

그 다음 `irssi` 프롬프트에서 서버에 접속합니다:

irssi -c localhost -p 6667 -n nickname

```text
/connect localhost 6667 mypassword
/connect localhost 6667 mypassword nickname
```

특정 닉네임으로 접속하고 싶다면:

```text
/connect -nick mynick localhost 6667 mypassword
```

접속 후 채널에 입장하고 채팅을 시작할 수 있습니다:

```text
/join #general
hello everyone
```

### 정리

```bash
make clean   # 오브젝트 파일 삭제
make fclean  # 오브젝트 파일 및 실행 파일 삭제
make re      # 프로젝트 재빌드
```

## 참고 자료

**IRC 프로토콜 문서:**

- [RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459) - Internet Relay Chat Protocol
- [RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812) - Internet Relay Chat: Client Protocol

**기술 참고 자료:**

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

**AI 사용:**
AI 도구는 다음과 같은 용도로 사용되었습니다:

- 개발 중 디버깅 및 오류 해결
- IRC 프로토콜 사양 및 예외 상황 이해
- 명령어 핸들러 및 메시지 파싱 로직에 대한 코드 리뷰 및 최적화 제안

보완할거
명령어 예시 정리해두기
quit 명령어 실행 시, 해당 클라이언트가 입장해있는 채널에 브로드캐스트

---

irssi에서는 /part를 진행해도 클라이언트에 해당 채널 창이 열려 있으면 "이미 채널에 존재한다."라고 판단해서 서버에 JOIN을 전송하지 않음
/wc를 입력하면 서버에 PART를 보내게 되는데 내부 동작은 정상적으로 진행됨.
irssi와 ft_irc 서버간의 호환문제

따라서 ft_irc에서는 /wc를 입력하거나 /part 후 /wc를 진행해주어야 함.

mode +k로 비밀번호 설정시
/join #general [key]

비밀번호가 틀렸거나 user limit으로 full일 때도 irssi는 창부터 바로 열기 때문에 /wc로 창을 꺼줘야지 기본 화면?으로 전환되면서 에러 메시지를 확인할 수 있음.

---

irssi 클라이언트로 서버에 접속하면 클라이언트가 서버한테 MODE +i를 보내는데 이건 user에 대한 mode라서 요구사항에 없기때문에 구현하지 않았음.

# 2_1

이번 단계에서는 client와 server의 역할을 하는 두 개의 프로그램을 만들어 client에서 요청을 보냈을 때 proxy server에서 child process를 만들어 hit/miss 여부를 판단해 web server에 request를 보내기 전까지의 작업을 수행하는 기능을 구현했습니다.
  
## 적용된 개념 정리
  
1. **socket**  
네트워크 socket은 컴퓨터 네트워크를 경유하는 프로세스간 통신의 종착점을 의미한다. 오늘날 컴퓨터간 통신의 대부분은 인터넷 프로토콜을 기반으로 두고 있으므로, 대부분의 네트워크 socket은 인터넷 socket이라고 할 수 있다.
2. **socket programming**  
TCP에 따른 server, client의 통신 과정은 다음과 같습니다.  
  
![tcp connection diagram](https://user-images.githubusercontent.com/68600592/177511357-6a3aed29-af9b-486d-944f-9c92bfb1da2a.jpg)  

3. **struct sockaddr_in** (socket 주소 정보를 담는 구조체)  
socket 주소에 대한 정보를 저장하는 구조체로 다음과 같은 members를 갖고 있습니다.  
```c
struct sockadd_in{
  short sin_family;
  u_short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
}
```
  
sin_family: 주소 체계  
sin_port: 16bits port number  
sin_addr: 32bits ip address  
sin_zero: struct sockaddr과의 호환성을 위한 dummy  

## 사용된 기술  
**socket**
```c
int socket(int domain, int type, int protocol);
```
Connection을 위한 시스템의 socket을 설정하기 위해 사용하는 함수입니다. socket 생성에 성공한 경우 양수를 그렇지 못한 경우 -1을 반환합니다.  
domain은 protocol의 형태, type: service type, protocol: 프로토콜(type 설정값에 따라 자동으로 결정됩니다.)  
  
**bind**
```c
int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
```
Socket과 sockaddr_in 구조체의 주소 정보를 binding 하기 위해 사용하는 함수입니다. 간단하게 말하면 생성한 소켓을 사용할 수 있도록 ip, port number를 할당하는 작업을 수행합니다.  
Binding에 성공한 경우 0을 그렇지 못한 경우 -1을 반환합니다.  
  
**listen**
```c
int listen(int sockfd, int backlog);
```
Server에서만 사용되는 함수로, client로 부터의 connection을 설정하기 위해 현재 process를 대기 상태로 변경합니다. connection 요청이 들어온 경우 대기 queue에 push합니다. 
  
**accept**
```c
int accept(int sockfd, struct sockader *cliaddr, socklen_t addrlen);
```
Server가 client의 접속을 받아들일 때 사용하는 함수입니다. listen() 함수에 의해 대기 queue에 push된 연결 요청들을 순차적으로 수락합니다. 성공한 경우 양수를 실패한 경우 -1을 반환합니다.  
  
**connect**
```c
int connect(int sockfd, const struct sockaddr* servaddr, socklen_t addrlen);
```
client에서 server와의 connection을 설정하기 위해서 사용합니다. connection 설정에 성공한 경우 양수를 반환하고, 실패한 경우 -1을 반환합니다.  
socket 함수를 이용해 얻은 socket의 file descriptor와 미리 정의한 server의 주소 정보를 담은 sockaddr 구조체를 인자로 받습니다.
  
**close**
```c
int close(int filedes);
```
파일 입출력에서도 사용하는 함수로 여기에서는 socket을 close하기 위해 사용했습니다.
  
**read/write**
```c
ssize_t write(int filedes, const void* buf, size_t nbytes);
ssize_t read(int filedes, void* buf, size_t nbytes);
```
일반적으로도 많이 사용하는 read, write 함수를 이용해 socket filedescriptor로 request, response를 주고 받습니다.
  
**htonl(), htons(), ntohl(), ntohs()**
```c
unsigned long int htonl(unsigned long int hostlong);
unsigned short int htons(unsigned short int hostshort);
unsigned long int ntohl(unsigned long int netlong);
unsigned short int ntohs(unsigned short int netshort);
```
CPU마다 host의 byte order가 다르기 때문에, 네트워크를 통한 전송 과정에서 byte order를 맞추는 과정이 필요한데, 이 때 사용하는 functions들로 각각의 함수들은 다음과 같은 경우에 사용합니다.
  
| name | operation|
| ----- | -------------------------------- |
| htonl | long data to network byte order  |
| htons | short data to network byte order |
| ntohl | long data to host byte order     |
| ntohs | short data to host byte order    |

  
**inet_addr()**
```c
unsigned long int inet_addr(const char* cp);
```
Dotted decimal type의 인터넷 host 주소를 network byte 순서인 이진 데이터 (32bit big-endian) 형태로 바꿔줍니다.  
입력이 유효한 경우 변환된 주소를 유효하지 않은 경우 -1을 반환합니다. 
  
+ little-endian: 하위 바이트가 앞에 표기되는 표기법, big-endian: 상위 바이트가 앞에 표기되는 표기법  

## 결과 화면

  

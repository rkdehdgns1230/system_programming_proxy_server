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
socket
```c

```
bind
```c
```
listen
```c
```
accept
```c

```
connect
```c

```
close
```c

```
read/write
```c

```
htonl(), htons(), ntohl(), ntohs()
```c

```
inet_addr()
```c

```
  
## 결과 화면
  

# System Programming Proxy Server
C언어와 우분투 시스템 콜을 이용한 프록시 서버 설계 및 구현

## Program Architecture  
![image](https://user-images.githubusercontent.com/68600592/211743267-e57668db-3286-4aa0-90ac-3badc1999fb7.png)

## 프록시 서버란??

프록시 서버 (proxy server)는 클라이언트가 자신을 통해 다른 네트워크 서비스에 간접적으로 접속할 수 있게 해주는 컴퓨터 시스템이나 응용 프로그램을 의미합니다.

서버(server)와 클라이언트(client) 사이에 중계기로서 대리로 통신을 수행하는 것을 가리켜 '프록시'라고 하며, 중계 기능을 하는 서버를 '프록시 서버'라고 합니다.

프록시 서버 중 일부는 프록시 서버에 요청된 내용을 캐시에 저장하여 이후에 중복된 요청에 대해서는 빠르게 처리할 수 있도록 저장하고, 같은 요청을 다시 하는 경우 proxy내의 cache에 저장되어 있는 자료들을 제공함으로써 자료를 빠르게 전송할 수 있습니다.

프로젝트의 주제가 바로 캐시에 최근에 가져온 자료를 저장해, 같은 자료를 요구하는 경우 proxy 내의 캐시에 있는 자료를 제공함으로써 자료를 빠르게 전송할 수 있는 프록시 서버입니다.

Client는 proxy를 사용해 자료를 받아오겠다는 사실을 host 컴퓨터에게 알려야 Proxy server를 통해 자료를 받아오는 것이 가능합니다.

HIT 케이스의 경우(프록시 서버에 caching된 경우) 전송 속도가 빠를 수 있지만, proxy server에 caching된 자료가 아닌 경우 전송이 더 오래 걸릴수도 있습니다. 

이는 캐싱된 데이터가 없는 경우 프록시 서버에 접근한 후 프록시 서버가 original 서버에 request를 보내 원하는 데이터를 받아올 때까지 이중으로 대기해야하기 때문입니다.

## 프로젝트 요구사항 정리
1. 프록시 서버 구현
    - 프록시 서버는 client(web browser)의 request를 받는다.
    - client request마다 서버 process에서 child process를 생성해 request를 handling 한다.
    - 프록시 서버는 캐시에 request에 맞는 캐싱 데이터가 있는지 확인한다.
    - HIT case(캐싱된 데이터가 있는 경우)에는 해당 데이터를 바로 client로 전송한다.
    - MISS case(캐싱된 데이터가 없는 경우)에는 original server에 request를 전송
    - original server의 response를 캐싱하고, client에게 전달한다.
2. 캐시 구현
    - 캐시에는 최근에 들어온 request에 대한 response message body를 모두 저장한다.
    - request의 URI를 hashing(SHA-1 사용) 해서 앞의 세 글자를 directory name으로 나머지 글자를 file name으로 설정
    - file안에 message body를 모두 저장한다.
3. 로그 기능
    - HIT, MISS 여부에 따라서 logfile에 log를 작성한다.
    - logfile은 shared resource 이므로 semaphore를 이용해 관리한다.
    - logfile에 log를 작성할 때에는 thread를 생성해 logging 작업을 수행하도록 구현한다.
    
## 프로젝트 결과
![image](https://user-images.githubusercontent.com/68600592/211743808-2937ad7b-bb38-442c-939b-67d4088c090c.png)

다음의 URL을 이용해 프록시 서버를 테스트 했습니다.
http://transcendentcoolinnerday.neverssl.com/online/

![image](https://user-images.githubusercontent.com/68600592/211744180-970d78f1-d982-488d-a823-4621a85b34ac.png)


# 1_3
proxy server가 사용자의 요청을 받는 작업과 caching 여부를 판단하는 작업을 동시에 수행하기 위해서는 multiprocessing 개념이 도입되어야 합니다.  
이 단계에서는 이를 위해 fork 함수를 이용해 사용자의 요청 처리를 위한 새로운 프로세스(sub process에 해당)를 생성하는 부분을 구현했습니다.  
  
**Main process (parent process)**  
사용자 요청 처리를 위한 child process를 생성및 관리  
  
**동작 방식**
  1. "[pid]Input CMD >>" string을 출력하고, 사용자의 명령어를 대기  
  2. "connect" 명령어 입력시 새로운 프로세스를 생성한 후 child process 종료시까지 대기  
  3. "quit" 명령어 입력시 동작 시간, 생성한 child processes 수 정보에 대한 log를 logfile에 작성 후 종료  
  
**Sub process (child process)**  
사용자의 URL을 입력 받고 지난 1_2에서 구현한 부분의 작업을 수행한다.
  
  1. "[pid]Input URL >> " string을 출력하고, 사용자의 url입력을 대기
  2. url이 입력되는 경우 1_2 구현 내용을 수행
  3. "bye"가 입력되는 경우 child process를 종료한다.

## 구현 내용
  
  1. 기존의 기능을 child process에서 수행하도록 코드 변경
  2. input command를 입력 받도록 구현
  3. process id를 logfile에 출력할 수 있도록 구현
  4. logfile.txt 작성 내용 부분 구현 수정

## 사용된 기술

```c
#include <sys/types.h>
#include <unistd.h>

pid_t getpid(void); // return the current process's process id
pid_t fork(void);   // return 0: child process, processID of child process: parent process, -1: error
pid_t wait(int *statloc); // return process id if ok, -1 an error
```
  
현재 process의 process id를 return하는 함수
```c
pid_t getpid(void);
```
  
child process를 만들기 위해 사용하는 함수  
2개의 return이 발생하는데, 0을 반환하는 경우 chlid process에 대한 반환값이고, process id를 반환하는 경우 parent process에 대한 반환값이다.
```c
pid_t fork(void);
```
  
child process의 종료를 대기하기 위해 사용하는 함수  
```c
pid_t wait(int *statloc);
```
  
## 결과 화면
  

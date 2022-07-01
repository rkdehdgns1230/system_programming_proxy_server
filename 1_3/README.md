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
  


## 구현 순서
  

## 사용된 기술

```c
#include <sys/types.h>
#include <unistd.h>

pid_t getpid(void); // return the current process's process id
pid_t fork(void);   // return 0: child process, processID of child process: parent process, -1: error
```

## 결과 화면
  

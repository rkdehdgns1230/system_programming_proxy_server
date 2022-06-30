# 1_2
해당 웹사이트에 대한 caching 여부를 판단하기 위해 HIT/MISS 여부를 판단하도록 구현하고, HIT/MISS 여부를  
logfile에 작성할 수 있도록 구현하는 단계이다.

## 구현 순서
1. time 함수를 이용해 시스템으로부터 현재 시간을 구한다.
2. logfile을 작성하기 위한 directory및 file path를 만든다.
3. directory name을 이용해 HIT/MISS 여부를 판단.
4. HIT/MISS에 따라서 logfile.txt에 log 작성
5. 프로그램 종료시 logfile.txt에 종료 정보를 작성

## 사용된 기술
**1. time function:**  
```c
time_t time(const time_t *timer)
```
UNIX kernel에 의해 제공되는 time service, 시스템의 현재 시간을 time_t type의 값으로 반환한다.  
time_t value는 1970년 1월 1일 00:00시를 기준으로 지난 초수를 표현한다.  

**2. localtime function:**
```c
struct tm *localtime(const time_t *timer)
```

time_t struct를 인자로 받아 tm struct type으로 시간 정보를 변환해서 반환하는 작업을 하는 함수이다.  
반환에 성공하는 경우 tm structure를 반환하고, 실패하는 경우 NULL을 반환한다.  

**3. struct tm:**  
```c
struct tm
{
  int tm_sec;   //초 [0, 59]
  int tm_min;   //분 [0, 59]
  int tm_hour;  //시 [0, 23]
  int tm_mday;  //일 [1, 31]
  int tm_mon;   //월 [0, 11]
  int tm_year;  //년 since 1900
  int tm_wday;  //요일, 일요일부터 [0, 6]
  int tm_yday;  //날짜 [0, 365]
  int tm_isdst; //Summer time flag <0, 0, >0
}
```

## Result Screen

**Logfile에 작성되는 내용**

HIT일 경우:  
```
[HIT]Directory name/filename-[Time information(year/month/day, hour:min:sec)]  
[HIT]URL-[Time information]  
```
MISS일 경우:  
```
[MISS]URL-[Time information]  
```
**프로그램 실행 화면**  

**logfile.txt 확인 결과**  

# 구현 목표

1. HTTP 요청을 Web server로 forwarding 하는 기능 구현
2. Signal handling 기능 추가
3. cache 추가 및 log 작성 기능

![image](https://user-images.githubusercontent.com/68600592/184096548-f44fddfe-57f7-4b75-a310-c2c6b1e981fa.png)

proxy server 작동 구조는 위와 같다.

![image](https://user-images.githubusercontent.com/68600592/184096639-733b3ba6-5d48-4d89-bd32-c1ba1ef37bbf.png)

이번 단계는 위 빨간 박스에 작성된 내용의 구현을 목표로 한다.


# 1_1

URL을 입력 받았을 때 이를 해시 함수(SHA1)를 이용해 고정된 길이의 데이터로 매핑해서 이를 사용해
디렉토리와 파일을 생성하는 단계

## 구현 순서
1. Input URL을 입력으로 받아 SHA1 함수를 이용해 hashed url로 변환.
2. getHomeDir 함수를 이용해 user의 home directory path를 얻는다.
3. home directory path 이용해 cache directory 생성
4. cache directory 내에 directory 및 file 생성

## 사용된 기술
**SHA1:** <br>
Cache에 저장하기 전 입력받은 URL을 hash하기 위해서 사용

**mkdir:** <br>
directory를 만드는 system call

## Dependencies
sudo apt-get install libssl-dev
compile하는 경우 -lcrpyto option 추가해야한다.

## Result Screen

<p align="center">
  <h1 align="center">Raspberry Pi 기반 웹 연동 1/100초 FND Stop Watch✨</h1>
</p>

## Index ⭐️
- [1. 프로젝트 목표](#1-프로젝트-목표)
- [2. 동작 구성](#2-동작-구성)
  - [2.1 `start.c`](#21-startc)
  - [2.2 `stop.c`, `clear.c`](#22-stopc-clearc)
- [3. 결과](#3-결과)

## 1. 프로젝트 목표
  
💡 **웹 서버 연동 1/100초 Stop Watch 작성 (FND 제어)**

- 웹 페이지는 3개의 버튼 존재 : **`Clear`, `Start`, `Stop`**
  - **`Clear`** : 0으로 초기화  
  - **`Start`** : 시작 또는 재시작 (1/100초)  
  - **`Stop`** : 정지  

- **힌트**
  
``` html
  <from method=get action="cgi-bin/start.cgi">
     <input type="submit" name="button" value="input">
  </from>
```
  - `start.cgi`는 child process(FND process)를 Fork하여 1/100초 Stop Watch로 FND를 제어
  - `stop.cgi`는 FND process에 Stop 메시지를 전달 (FIFO 사용)
  - `clear.cgi`는 FND process에 Clear 메시지를 전달 (FIFO 사용)
  - FND process는 FND를 제어하는 Thread와 FIFO로 IPC를 수행하는 Thread로 구성

## 2. 동작 구성
- `stopwatch.html', 'start.c', 'stop.c', 'clear.c'

### 2.1 `start.c`
- 'main()', 'FndSelect()', 'FndDisplay()', 'fndThread()', 'fifoThread()', Init()' 으로 구성.
- 'FndSelect()', 'FndDisplay()': 강의자료와 같은 함수를 사용. (Lab 2-3 참고.)
- 'fndThread()', 'fifoThread()': 'pthread'에 의해서 Multi-thread로 작동.
- 'Init()': GPIO 초기설정.
- 'main()'
  - 실행되고 stopwatch가 동작할 수 있도록 Multi-thread 준비
  - 실행 종료 후 Start 데이터 받음. 이는 FIFO파일을 생성한 뒤, 해당 파일에 클라이언트로 전달받은 Start, Stop, Clear 저장. 그리고 `fork()`를 통해 Child Process 생성. Child Process에서는 `fndThread()`와 `fifoThread()`로 Thread 생성 뒤, FIFO파일에 Start 출력하고 종료. 이후에 `start.c`가 실행될 때, Start 버튼이 눌렀을 때로 FIFO 파일에 "Start"를 출력하는 역할만 수행하게 됩니다.
- `fifoThread()`: Multi-thread로 작동. 실행되면 While문을 돌면서 FIFO 파일을 열고, 전역변수 `state`를 Start일 때는 1, Stop일 때는 0, Clear일 때는 -1로 변경.
- 'fndThread()': 'fifoThread()'와 마찬가지로 Multi-thread로 작동. 실행되면 While문을 돌면서 `state` 값에 따라 1/100 Fnd Stop Watch를 작동시킴. (이는 Lab2-4 실습과제(2)와 같은 원리로 작성.) `state`가 -1일 시, Break를 통해 While문을 나가면서 FIFO파일을 삭제시킵니다.

### 2.2 `stop.c`, `clear.c`
&nbsp;RaspberryPi아이피/stopWatch.html에서 버튼이 눌러졌을 때 실행되고, 각 버튼에 맞는 Message를 FIFO파일에 출력합니다. <br/>
FFO파일 출력에 문제가 있을 시, 확인할 수 있도록 구현하였습니다. <br/>

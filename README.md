<p align="center">
  <h1 align="center">Raspberry Pi 기반 웹 연동 1/100초 FND Stop Watch✨</h1>
</p>

## Index ⭐️
- [1. 프로젝트 목표](#1-프로젝트-목표)
- [2. 동작 구성](#2-동작-구성)
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
  - `start.cgi`는 child process(FND process)를 fork하여 1/100초 stop watch로 FND를 제어
  - `stop.cgi`는 FND process에 stop 메시지를 전달 (FIFO 사용)
  - `clear.cgi`는 FND process에 clear 메시지를 전달 (FIFO 사용)
  - FND process는 FND를 제어하는 thread와 FIFO로 IPC를 수행하는 thread로 구성

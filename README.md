# OS 과제 2

## Dependency
```bash
# For Ubuntu, Debian
sudo apt-get install gcc cmake
``` 

## Build
```bash
mkdir build
cd build
cmake ..
make
```

## Usage
```bash
# [Program] [Config] [Log]
./fcfs ../config ../log
./sjf ../config ../log
```

## Config
첫 줄에 실행할 명령의 개수
둘째 줄부터 실행할 명령을 순서대로 적어주면 된다. 
```
5
./process1 3
./process1 6
./process1 9
./process1 5
./process1 1
```

## Log
최초 시작 시에 빈 log 파일이 반드시 존재해야만 한다. 

## How it works
* 실제 프로그램에 대한 스케쥴링을 시뮬레이션한다. 
* FCFS의 경우 Config에 입력된 순서대로 실행한다. 
* SJF의 경우 기존 Log를 파싱하여 Burst time에 대한 Exponential Average를 취하여 우선순위를 정한다. (SMOOTHING_FACTOR는 0.5)
로그가 쌓여야 Burst time 추정이 가능하기 때문에 FCFS를 수행한 이후에 SJF를 수행하기를 추천한다. 
* RR의 경우 SIGSTOP을 통해 Time Quantum 마다 프로세스를 멈춘다. Time Quantum은 기본값은 0.05s 이지만 로그를 통해 Burst Time을 추정하여 (SJF와 같은 계산 방식) Thumb-rule에 의한
최적 Time Quantum을 구할 수 있다. 따라서 마찬가지로 FCFS를 수 회 수행한 후 SJF를 수행하기를 추천한다. 

## Limitation
* 실제 프로세스를 기반으로 하기 때문에 Child Process에서 Fork가 일어나는 경우 해당 Child-Child Process는 관리가 불가능하다. 

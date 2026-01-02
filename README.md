# study_c

## 도커 빌드 명령어
### docker build
docker build -t c-dev .
### docker run
docker run -it --name c-dev-container -v ${PWD}:/app c-dev

## 컴파일러 빌드 명령어
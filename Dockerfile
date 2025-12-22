FROM debian:bookworm-slim

# C 개발에 필요한 최소 도구
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    gdb \
    vim \
    curl \
    clang-format \
    && rm -rf /var/lib/apt/lists/*

# 작업 디렉토리
WORKDIR /app

# 컨테이너가 바로 꺼지지 않도록 bash 실행
CMD ["bash"]

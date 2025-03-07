FROM ubuntu:24.04

WORKDIR /root/app 

RUN apt update -y
RUN apt install -y libgtest-dev cmake ninja-build build-essential clang 

CMD ["./build.sh"]

FROM ubuntu:22.04 as builder

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends \
    wget software-properties-common
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
RUN add-apt-repository 'deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main'
RUN apt update && apt install -y cmake ninja-build flex bison libfl-dev clang-16 libc++-16-dev libc++abi-16-dev git

WORKDIR /dbuf

ENTRYPOINT ["/bin/bash"]

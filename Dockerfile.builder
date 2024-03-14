FROM ubuntu:22.04 as builder

# Set environment variables for non-interactive installation and use clang-16
ENV DEBIAN_FRONTEND=noninteractive \
    CC=clang-16 \
    CXX=clang++-16

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    wget \
    software-properties-common \
    cmake \
    ninja-build \
    flex \
    bison \
    libfl-dev \
    git \
    ccache \
    clang-format \
    clang-tidy && \
    wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc && \
    add-apt-repository 'deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main' && \
    apt-get update && \
    apt-get install -y \
    clang-16 \
    libc++-16-dev \
    libc++abi-16-dev

WORKDIR /dbuf

ENTRYPOINT ["/bin/bash"]

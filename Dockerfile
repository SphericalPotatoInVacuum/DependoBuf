FROM ubuntu:22.04 as builder

ARG BUILD_TYPE=Release
ARG JOBS=1
ENV CC=clang-16
ENV CXX=clang++-16

# Install dependencies
RUN apt update && DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends \
    wget software-properties-common
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
RUN add-apt-repository 'deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main'
RUN apt update && apt install -y cmake ninja-build flex bison libfl-dev clang-16 libc++-16-dev libc++abi-16-dev git

COPY lib lib
COPY test test
COPY src src

COPY CMakeLists.txt ./

RUN mkdir build
RUN cmake -B build -S . -G "Ninja" \
    -DZ3_BUILD_LIBZ3_SHARED=FALSE \
    -DBUILD_SHARED_LIBS=FALSE \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++"
RUN cmake --build build --parallel $JOBS --config $BUILD_TYPE

FROM scratch as runner

WORKDIR /dbuf
COPY --from=builder /build/src/dbuf ./

ENTRYPOINT ["/dbuf/dbuf"]

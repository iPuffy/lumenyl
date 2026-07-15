FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build tools
RUN apt-get update && apt-get install -y \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ninja-build \
    build-essential \
    cmake \
 && rm -rf /var/lib/apt/lists/*

# Install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
 && /opt/vcpkg/bootstrap-vcpkg.sh

WORKDIR /app

# Copy files that rarely change
COPY CMakeLists.txt .
COPY vcpkg.json .
COPY src ./src
COPY public ./public

# Configure and build
RUN cmake -S . -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake

RUN cmake --build build -j

EXPOSE 10000

CMD ["./build/lumenyl"]
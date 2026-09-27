# ==========================================
# Stage 1: Build React frontend
# ==========================================

FROM node:22 AS frontend

WORKDIR /app/frontend

COPY frontend/package.json frontend/package-lock.json ./

RUN npm ci

COPY frontend/ ./

RUN npm run build


# ==========================================
# Stage 2: Build Drogon backend
# ==========================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_MAX_CONCURRENCY=2

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

# Copy backend files
COPY CMakeLists.txt .
COPY vcpkg.json .
COPY src ./src

# Copy React production build
COPY --from=frontend /app/frontend/dist ./public

# Configure CMake
RUN cmake -S . -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_BUILD_TYPE=release

# Build backend
RUN cmake --build build -j2

EXPOSE 10000

CMD ["./build/lumenyl"]
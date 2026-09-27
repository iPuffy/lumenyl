# ==========================================
# Stage 1: Build React frontend
# ==========================================

FROM node:22 AS frontend

WORKDIR /app/frontend

COPY FRONTEND/package.json FRONTEND/package-lock.json ./

RUN npm ci

COPY FRONTEND/ ./

RUN npm run build


# ==========================================
# Stage 2: Build Drogon backend
# ==========================================

FROM ubuntu:24.04 AS backend

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

# Copy backend files
COPY CMakeLists.txt .
COPY vcpkg.json .
COPY src ./src

# Copy the React production build into Drogon's public directory
COPY --from=frontend /app/frontend/dist ./public

# Configure
RUN cmake -S . -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
RUN cmake --build build -j

EXPOSE 10000

CMD ["./build/lumenyl"]
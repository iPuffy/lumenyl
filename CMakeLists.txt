cmake_minimum_required(VERSION 3.20)

project(lumenyl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)

add_executable(
    lumenyl
    src/main.cpp
    src/websocket/ChatSocket.cpp
    src/services/MatchmakingService.cpp
)

target_link_libraries(lumenyl PRIVATE Drogon::Drogon)
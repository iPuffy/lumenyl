#pragma once

#include <drogon/WebSocketConnection.h>

#include <vector>
#include <mutex>
#include <unordered_map>
#include <queue>

using namespace drogon;

class MatchmakingService
{
public:

    static MatchmakingService& instance();

    void addClient
    (
        const WebSocketConnectionPtr& connection
    );

    void removeClient
    (
        const WebSocketConnectionPtr& connection
    );

    void handleMessage
    (
        const WebSocketConnectionPtr& sender,
        const std::string& message
    );

private:

    void matchNextUser();

    std::mutex mutex;

    std::queue<WebSocketConnectionPtr> waitingQueue;

    std::unordered_map<
        WebSocketConnectionPtr,
        int
    > matches;

    struct Lobby
    {
        std::vector<WebSocketConnectionPtr> users;
    };

    std::vector<Lobby> lobbies;
    std::queue<int> emptyLobbies;
};
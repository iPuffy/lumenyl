#pragma once

#include <drogon/WebSocketConnection.h>

#include <vector>
#include <mutex>
#include <unordered_map>
#include <queue>
#include <json/json.h>

using namespace drogon;

class Matchmaking_Service
{
private:

    std::mutex mutex;

    std::queue<WebSocketConnectionPtr> listener_queue;
    std::queue<WebSocketConnectionPtr> talker_queue;

    std::unordered_map<
        WebSocketConnectionPtr,
        int
    > matches;

    std::unordered_map<
        WebSocketConnectionPtr,
        std::string
    > client_roles;

    /// Lobbies

    struct Lobby
    {
        std::vector<WebSocketConnectionPtr> users;
    };

    std::vector<Lobby> lobbies;
    std::queue<int> empty_lobbies;

    std::unordered_map<
        WebSocketConnectionPtr,
        Json::Value
    > public_keys;

public:

    static Matchmaking_Service& instance();

    void match_next_user();

    void remove_from_queue
    (
        const WebSocketConnectionPtr& connection
    );

    void add_client
    (
        const WebSocketConnectionPtr& connection,
        const std::string& role
    );

    void remove_client
    (
        const WebSocketConnectionPtr& connection
    );

    void cancel_matchmaking
    (
        const WebSocketConnectionPtr& connection
    );

    void leave_lobby
    (
        const WebSocketConnectionPtr& connection
    );

    void handle_message
    (
        const WebSocketConnectionPtr& sender,
        const std::string& message
    );

    void store_public_key
    (
        const WebSocketConnectionPtr& connection,
        const Json::Value& public_key
    );
};
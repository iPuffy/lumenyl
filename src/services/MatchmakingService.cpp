#include "MatchmakingService.h"

#include <algorithm>

MatchmakingService& MatchmakingService::instance()
{
    static MatchmakingService service;
    return service;
}

void MatchmakingService::addClient(const WebSocketConnectionPtr& connection)
{
    {
        std::lock_guard<std::mutex> lock(mutex);

        waitingQueue.push(connection);
    }

    matchNextUser();
}

void MatchmakingService::removeClient(const WebSocketConnectionPtr& connection)
{
    {
        std::lock_guard<std::mutex> lock(mutex);

        auto it = matches.find(connection);
        if (it != matches.end())
        {
            int lobby = it->second;

            lobbies[lobby].users.erase
            (
                std::remove
                (
                    lobbies[lobby].users.begin(),
                    lobbies[lobby].users.end(),
                    connection
                ),
                lobbies[lobby].users.end()
            );
            matches.erase(connection);

            for (const auto& client : lobbies[lobby].users)
            {
                client->send("A user disconnected from your lobby.");
            }

            if (lobbies[lobby].users.size() == 1)
            {
                auto last_user = lobbies[lobby].users[0];
                matches.erase(last_user);
                lobbies[lobby].users.clear();
                emptyLobbies.push(lobby);

                last_user->send("You were alone in the lobby and have been queued up for another lobby.");

                if (last_user && last_user->connected())
                {
                    waitingQueue.push(last_user);
                }
            }
            else if (lobbies[lobby].users.empty())
            {
                emptyLobbies.push(lobby);
            }
        }
    }

    matchNextUser();
}

void MatchmakingService::matchNextUser()
{
    std::lock_guard<std::mutex> lock(mutex);

    std::queue<WebSocketConnectionPtr> cleaned;

    while (!waitingQueue.empty())
    {
        auto user = waitingQueue.front();
        waitingQueue.pop();

        if (user && user->connected())
        {
            cleaned.push(user);
        }  
    }

    waitingQueue.swap(cleaned);

    while (waitingQueue.size() >= 2)
    {
        const WebSocketConnectionPtr& user_A = waitingQueue.front();
        waitingQueue.pop();

        const WebSocketConnectionPtr& user_B = waitingQueue.front();
        waitingQueue.pop();

        int chosenLobby;

        if (!emptyLobbies.empty())
        {
            chosenLobby = emptyLobbies.front();
            emptyLobbies.pop();
        }
        else
        {
            lobbies.push_back({});

            chosenLobby = static_cast<int>(lobbies.size()) - 1;
        }

        lobbies[chosenLobby].users.push_back(user_A);
        lobbies[chosenLobby].users.push_back(user_B);

        matches[user_A] = chosenLobby;
        matches[user_B] = chosenLobby;

        user_A->send("Matched!");
        user_B->send("Matched!");
    }
}

void MatchmakingService::handleMessage(const WebSocketConnectionPtr& sender, const std::string& message)
{
    std::vector<WebSocketConnectionPtr> receivers;

    {
        std::lock_guard<std::mutex> lock(mutex);

        auto it = matches.find(sender);
        if (it == matches.end())
        {
            return;
        }

        int sender_lobby = it->second;

        for (const auto& user : lobbies[sender_lobby].users)
        {
            if (user == sender)
            {
                continue;
            }

            receivers.push_back(user);
        }
    }

    for (auto& user : receivers)
    {
        if (user && user->connected())
        {
            user->send(message);
        }
    }
}
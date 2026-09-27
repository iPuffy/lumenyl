#include "MatchmakingService.h"

#include <algorithm>
#include <iostream>
#include <json/json.h>

Matchmaking_Service& Matchmaking_Service::instance()
{
    static Matchmaking_Service service;
    return service;
}

void Matchmaking_Service::remove_from_queue
(
    const WebSocketConnectionPtr& connection
)
{
    std::queue<WebSocketConnectionPtr> cleaned_listeners;

    while (!listener_queue.empty())
    {
        auto user = listener_queue.front();
        listener_queue.pop();

        if (user != connection)
        {
            cleaned_listeners.push(user);
        }
    }

    listener_queue.swap(cleaned_listeners);

    std::queue<WebSocketConnectionPtr> cleaned_talkers;

    while (!talker_queue.empty())
    {
        auto user = talker_queue.front();
        talker_queue.pop();

        if (user != connection)
        {
            cleaned_talkers.push(user);
        }
    }

    talker_queue.swap(cleaned_talkers);
}

void Matchmaking_Service::add_client
(
    const WebSocketConnectionPtr& connection,
    const std::string& role
)
{
    {
        std::lock_guard<std::mutex> lock(mutex);

        client_roles[connection] = role;

        if (role == "listener")
        {
            listener_queue.push(connection);
        }
        else if (role == "talker")
        {
            talker_queue.push(connection);
        }

        std::cout
            << "Added " << role
            << " | listeners: " << listener_queue.size()
            << " | talkers: " << talker_queue.size()
            << std::endl;
    }

    match_next_user();
}

void Matchmaking_Service::remove_client
(
    const WebSocketConnectionPtr& connection
)
{
    {
        std::lock_guard<std::mutex> lock(mutex);

        remove_from_queue(connection);

        auto match = matches.find(connection);

        if (match == matches.end())
        {
            client_roles.erase(connection);

            return;
        }

        int lobby = match->second;

        matches.erase(connection);

        auto& users = lobbies[lobby].users;

        users.erase
        (
            std::remove
            (
                users.begin(),
                users.end(),
                connection
            ),
            users.end()
        );

        Json::Value response;

        response["type"] = "system";
        response["message"] = "A user left your lobby.";

        Json::StreamWriterBuilder builder;

        for (const auto& user : users)
        {
            if (user && user->connected())
            {
                user->send(
                    Json::writeString(builder, response)
                );
            }
        }

        if (users.size() == 1)
        {
            auto remaining_user = users[0];

            matches.erase(remaining_user);

            std::string role = client_roles[remaining_user];

            users.clear();

            empty_lobbies.push(lobby);

            if (remaining_user && remaining_user->connected())
            {
                if (role == "listener")
                {
                    listener_queue.push(remaining_user);
                }
                else if (role == "talker")
                {
                    talker_queue.push(remaining_user);
                }

                Json::Value response;

                response["type"] = "system";
                response["message"] =
                    "You were alone in the lobby and have been queued up for another lobby.";

                Json::StreamWriterBuilder builder;

                remaining_user->send(
                    Json::writeString(builder, response)
                );
            }
        }
        else if (users.empty())
        {
            empty_lobbies.push(lobby);
        }

        client_roles.erase(connection);
        public_keys.erase(connection);
    }

    match_next_user();
}

void Matchmaking_Service::cancel_matchmaking
(
    const WebSocketConnectionPtr& connection
)
{
    {
        std::lock_guard<std::mutex> lock(mutex);

        remove_from_queue(connection);
        client_roles.erase(connection);
    }

    match_next_user();
}

void Matchmaking_Service::leave_lobby
(
    const WebSocketConnectionPtr& connection
)
{
    WebSocketConnectionPtr remaining_user;

    {
        std::lock_guard<std::mutex> lock(mutex);

        auto match = matches.find(connection);

        if (match == matches.end())
        {
            remove_from_queue(connection);
            client_roles.erase(connection);
        }
        else
        {
            int lobby = match->second;

            matches.erase(connection);

            auto& users = lobbies[lobby].users;

            users.erase
            (
                std::remove(
                    users.begin(),
                    users.end(),
                    connection
                ),
                users.end()
            );

            if (users.size() == 1)
            {
                remaining_user = users[0];

                matches.erase(remaining_user);

                std::string role =
                    client_roles[remaining_user];

                users.clear();

                empty_lobbies.push(lobby);

                if (remaining_user && remaining_user->connected())
                {
                    if (role == "listener")
                    {
                        listener_queue.push(remaining_user);
                    }
                    else if (role == "talker")
                    {
                        talker_queue.push(remaining_user);
                    }
                }
            }
            else if (users.empty())
            {
                empty_lobbies.push(lobby);
            }

            client_roles.erase(connection);
            public_keys.erase(connection);
        }
    }

    if (remaining_user && remaining_user->connected())
    {
        Json::Value response;

        response["type"] = "system";
        response["event"] = "partner_left";
        response["message"] = "A user left your lobby.";


        Json::StreamWriterBuilder builder;

        remaining_user->send(
            Json::writeString(builder, response)
        );

        response["event"] = "queued";
        response["message"] =
            "You were alone in the lobby and have been queued up for another lobby.";

        remaining_user->send(
            Json::writeString(builder, response)
        );
    }

    match_next_user();
}

void Matchmaking_Service::match_next_user()
{
    std::lock_guard<std::mutex> lock(mutex);

    std::cout
        << "Trying to match"
        << " | listeners: " << listener_queue.size()
        << " | talkers: " << talker_queue.size()
        << std::endl;

    while (!listener_queue.empty() && !talker_queue.empty())
    {
        const auto& listener = listener_queue.front();
        listener_queue.pop();

        const auto& talker = talker_queue.front();
        talker_queue.pop();

        int chosen_lobby;

        if (!empty_lobbies.empty())
        {
            chosen_lobby = empty_lobbies.front();
            empty_lobbies.pop();
        }
        else
        {
            lobbies.push_back({});

            chosen_lobby =
                static_cast<int>(lobbies.size()) - 1;
        }

        lobbies[chosen_lobby].users.push_back(listener);
        lobbies[chosen_lobby].users.push_back(talker);

        matches[listener] = chosen_lobby;
        matches[talker] = chosen_lobby;

        std::cout << "MATCH FOUND!" << std::endl;

        Json::Value response;

        response["type"] = "system";
        response["event"] = "matched";
        response["message"] = "You've been matched with someone.";


        Json::StreamWriterBuilder builder;

        std::string json_message =
            Json::writeString(builder, response);

        listener->send(json_message);
        talker->send(json_message);

        if (
            public_keys.contains(listener) &&
            public_keys.contains(talker)
            )
        {
            Json::StreamWriterBuilder builder;

            Json::Value listener_key_message;

            listener_key_message["type"] =
                "key_exchange";

            listener_key_message["public_key"] =
                public_keys[talker];

            listener->send(
                Json::writeString(
                    builder,
                    listener_key_message
                )
            );

            Json::Value talker_key_message;

            talker_key_message["type"] =
                "key_exchange";

            talker_key_message["public_key"] =
                public_keys[listener];

            talker->send(
                Json::writeString(
                    builder,
                    talker_key_message
                )
            );
        }
    }
}

void Matchmaking_Service::handle_message
(
    const WebSocketConnectionPtr& sender,
    const std::string& message
)
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

void Matchmaking_Service::store_public_key
(
    const WebSocketConnectionPtr& connection,
    const Json::Value& public_key
)
{
    std::lock_guard<std::mutex> lock(mutex);

    public_keys[connection] = public_key;
}
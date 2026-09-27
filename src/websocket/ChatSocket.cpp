#include "ChatSocket.h"
#include "../services/MatchmakingService.h"

#include <iostream>
#include <json/json.h>
#include <sstream>

void Chat_Socket::handleNewMessage
(
    const WebSocketConnectionPtr& wsConn,
    std::string&& message,
    const WebSocketMessageType& type
)
{
    if (type != WebSocketMessageType::Text)
    {
        return;
    }

    std::cout << "Received: " << message << std::endl;

    Json::Value data;
    Json::CharReaderBuilder reader;
    std::string errors;

    std::istringstream stream(message);

    if (Json::parseFromStream(
        reader,
        stream,
        &data,
        &errors))
    {
        if (data["type"] == "key_exchange")
        {
            Matchmaking_Service::instance().store_public_key(
                wsConn,
                data["public_key"]
            );

            return;
        }

        if (data["type"] == "encrypted")
        {
            Matchmaking_Service::instance().handle_message(
                wsConn,
                message
            );

            return;
        }
    }

    if (message == "listener" || message == "talker")
    {
        {
            std::lock_guard<std::mutex> lock(mutex);

            registered_clients.insert(wsConn);
        }

        Matchmaking_Service::instance().add_client(
            wsConn,
            message
        );

        return;
    }

    if (message == "cancel")
    {
        Matchmaking_Service::instance().cancel_matchmaking(
            wsConn
        );

        return;
    }

    if (message == "leave")
    {
        Matchmaking_Service::instance().leave_lobby(
            wsConn
        );

        return;
    }

    Matchmaking_Service::instance().handle_message(
        wsConn,
        message
    );
}

void Chat_Socket::handleNewConnection
(
	const HttpRequestPtr& req,
	const WebSocketConnectionPtr& wsConn
)
{
	std::cout << "Client connected!" << std::endl;
}

void Chat_Socket::handleConnectionClosed
(
	const WebSocketConnectionPtr& wsConn
)
{
	{
		std::lock_guard<std::mutex> lock(mutex);

		registered_clients.erase(wsConn);
	}

	Matchmaking_Service::instance().remove_client(wsConn);

	std::cout << "Client disconnected!" << std::endl;
}
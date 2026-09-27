#include "ChatSocket.h"
#include "../services/MatchmakingService.h"

#include <iostream>

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
        Matchmaking_Service::instance().cancel_matchmaking(wsConn);

        return;
    }

    if (message == "leave")
    {
        Matchmaking_Service::instance().leave_lobby(wsConn);

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
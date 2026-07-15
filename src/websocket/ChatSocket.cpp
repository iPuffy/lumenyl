#include "ChatSocket.h"
#include "../services/MatchmakingService.h"

#include <iostream>

void ChatSocket::handleNewMessage
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

	MatchmakingService::instance().handleMessage(wsConn, message);
}

void ChatSocket::handleNewConnection
(
	const HttpRequestPtr& req,
	const WebSocketConnectionPtr& wsConn
)
{
	MatchmakingService::instance().addClient(wsConn);

	std::cout << "Client connected!" << std::endl;
}

void ChatSocket::handleConnectionClosed
(
	const WebSocketConnectionPtr& wsConn
)
{
	MatchmakingService::instance().removeClient(wsConn);

	std::cout << "Client disconnected!" << std::endl;
}
#pragma once

#include <drogon/WebSocketController.h>

#include <mutex>
#include <unordered_set>

using namespace drogon;

class Chat_Socket : public WebSocketController<Chat_Socket>
{
private:

	std::mutex mutex;

	std::unordered_set<WebSocketConnectionPtr> registered_clients;

public:

	WS_PATH_LIST_BEGIN
		WS_PATH_ADD("/chat");
	WS_PATH_LIST_END

	void handleNewMessage
	(
		const WebSocketConnectionPtr& wsConn,
		std::string&& message,
		const WebSocketMessageType& type
	) override;

	void handleNewConnection
	(
		const HttpRequestPtr& req,
		const WebSocketConnectionPtr& wsConn
	) override;

	void handleConnectionClosed
	(
		const WebSocketConnectionPtr& wsConn
	) override;
};
#pragma once

#include <drogon/WebSocketController.h>

using namespace drogon;

class ChatSocket : public WebSocketController<ChatSocket>
{
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
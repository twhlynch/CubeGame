#include "WebSocketServer.hpp"

#include <atomic>

#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXWebSocket.h"

bool WebSocketServer::Start(uint16_t port)
{
	if (_running)
	{
		Stop();
	}

	ix::initNetSystem();

	_port = port;
	_server = std::make_unique<ix::WebSocketServer>(port, "0.0.0.0");

	_server->setOnConnectionCallback(
		[this](
			const std::weak_ptr<ix::WebSocket> &ws,
			const std::shared_ptr<ix::ConnectionState> &) {
			auto s = ws.lock();
			if (!s) { return; }

			static std::atomic<uint32_t> s_nextID{1};
			uint32_t id = s_nextID.fetch_add(1);

			s->setOnMessageCallback([this, id](const ix::WebSocketMessagePtr &msg) {
				switch (msg->type)
				{
					case ix::WebSocketMessageType::Open:
						if (_onConnect)
						{
							_onConnect(id);
						}
						break;
					case ix::WebSocketMessageType::Close:
					case ix::WebSocketMessageType::Error:
						if (_onDisconnect)
						{
							_onDisconnect(id);
						}
						break;
					case ix::WebSocketMessageType::Message:
						if (_onMessage)
						{
							_onMessage(id, msg->str);
						}
						break;
					default:
						break;
				}
			});

			s->start();
		});

	if (!_server->listenAndStart())
	{
		_server.reset();
		return false;
	}

	_running = true;
	return true;
}

void WebSocketServer::Stop()
{
	if (!_running) { return; }

	_running = false;

	if (_server)
	{
		_server->stop();
		_server.reset();
	}
}

void WebSocketServer::Broadcast(const std::string &text)
{
	if (!_server) { return; }

	for (const auto &ws : _server->getClients())
	{
		ws->sendText(text);
	}
}

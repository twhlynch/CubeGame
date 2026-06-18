#pragma once

#include <ixwebsocket/IXWebSocketServer.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace CG
{

class WebSocketServer
{
public:
	using MessageCallback = std::function<void(uint32_t clientID, const std::string &message)>;
	using EventCallback = std::function<void(uint32_t clientID)>;

	WebSocketServer() = default;
	~WebSocketServer() { Stop(); }

	WebSocketServer(const WebSocketServer &) = delete;
	WebSocketServer &operator=(const WebSocketServer &) = delete;

	bool Start(uint16_t port);
	void Stop();
	[[nodiscard]] bool IsRunning() const { return _running; }
	[[nodiscard]] uint16_t GetPort() const { return _port; }

	void Broadcast(const std::string &text);

	void SetOnMessage(MessageCallback cb) { _onMessage = std::move(cb); }
	void SetOnConnect(EventCallback cb) { _onConnect = std::move(cb); }
	void SetOnDisconnect(EventCallback cb) { _onDisconnect = std::move(cb); }

	static std::string GetLocalIP();

private:
	uint16_t _port = 0;
	bool _running = false;

	std::unique_ptr<ix::WebSocketServer> _server;

	MessageCallback _onMessage;
	EventCallback _onConnect;
	EventCallback _onDisconnect;
};

} // namespace CG

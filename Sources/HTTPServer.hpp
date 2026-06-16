#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "ixwebsocket/IXHttpServer.h"

class HTTPServer
{
public:
	HTTPServer() = default;
	~HTTPServer() { Stop(); }

	HTTPServer(const HTTPServer &) = delete;
	HTTPServer &operator=(const HTTPServer &) = delete;

	bool Start(uint16_t port, const std::string &rootDirectory);
	void Stop();
	[[nodiscard]] bool IsRunning() const { return _running; }
	[[nodiscard]] uint16_t GetPort() const { return _port; }
	static const char *GetMimeType(const std::string &path);

private:
	uint16_t _port = 0;
	bool _running = false;
	std::unique_ptr<ix::HttpServer> _server;
};

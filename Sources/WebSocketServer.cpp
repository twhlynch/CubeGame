#include "WebSocketServer.hpp"

#include <RayneConfig.h>
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
			const std::shared_ptr<ix::ConnectionState> &
		) {
			auto s = ws.lock();
			if (!s) { return; }

			static std::atomic<uint32_t> s_nextID {1};
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
		}
	);

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
		if (ws->getReadyState() == ix::ReadyState::Open)
		{
			ws->sendText(text);
		}
	}
}

std::string WebSocketServer::GetLocalIP()
{
	std::string ip = "127.0.0.1";

	// UDP socket trick
	// connect to 8.8.8.8:53 then read its IP with getsockname
#if RN_PLATFORM_ANDROID

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd >= 0)
	{
		struct sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(53);
		inet_pton(AF_INET, "8.8.8.8", &addr.sin_addr);

		if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
		{
			struct sockaddr_in local = {};
			socklen_t len = sizeof(local);
			if (getsockname(fd, (struct sockaddr *)&local, &len) == 0)
			{
				std::array<char, INET_ADDRSTRLEN> buf;
				if (ix::inet_ntop(AF_INET, &local.sin_addr, buf.data(), buf.size()))
				{
					ip = buf.data();
				}
			}
		}
		close(fd);
	}

	if (ip != "127.0.0.1") { return ip; }
#endif
	// resolve local hostname
	// then find non-loopback IPv4 address
	// NOTE: untested on Windows

	std::array<char, 256> hostname;
	if (gethostname(hostname.data(), hostname.size()) != 0) { return ip; }

	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;

	struct addrinfo *result = nullptr;
	int err = getaddrinfo(hostname.data(), nullptr, &hints, &result);
	if (err != 0 || !result) { return ip; }

	for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next)
	{
		if (rp->ai_family != AF_INET) { continue; }

		auto *sa = reinterpret_cast<sockaddr_in *>(rp->ai_addr);
		uint32_t addr = ntohl(sa->sin_addr.s_addr);
		if ((addr & 0xFF000000) == 0x7F000000) { continue; }

		std::array<char, INET_ADDRSTRLEN> buf;
		if (ix::inet_ntop(AF_INET, &sa->sin_addr, buf.data(), buf.size()))
		{
			ip = buf.data();
			break;
		}
	}

	freeaddrinfo(result);

	return ip;
}

#include "HTTPServer.hpp"

#include <fstream>
#include <sstream>

#include "ixwebsocket/IXHttpServer.h"
#include "ixwebsocket/IXNetSystem.h"

bool HTTPServer::Start(uint16_t port, const std::string &rootDirectory)
{
	if (_running) { Stop(); }

	ix::initNetSystem();

	_port = port;
	_server = std::make_unique<ix::HttpServer>(port, "0.0.0.0");

	_server->setOnConnectionCallback(
		[rootDirectory](const ix::HttpRequestPtr &request, const std::shared_ptr<ix::ConnectionState> &) {
			std::string uri = request->uri;
			if (uri.empty() || uri == "/") { uri = "/index.html"; }

			std::string path = rootDirectory + uri;

			std::ifstream file(path, std::ios::binary);
			if (!file.is_open())
			{
				return std::make_shared<ix::HttpResponse>(
					404, "Not Found", ix::HttpErrorCode::Ok, ix::WebSocketHttpHeaders(), std::string()
				);
			}

			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string content = buffer.str();

			ix::WebSocketHttpHeaders headers;
			headers["Server"] = "CubeGame";
			headers["Content-Type"] = GetMimeType(uri);
			headers["Accept-Ranges"] = "none";

			return std::make_shared<ix::HttpResponse>(200, "OK", ix::HttpErrorCode::Ok, headers, content);
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

void HTTPServer::Stop()
{
	if (!_running) { return; }

	_running = false;

	if (_server)
	{
		_server->stop();
		_server.reset();
	}
}

const char *HTTPServer::GetMimeType(const std::string &path)
{
	size_t dot = path.rfind('.');
	if (dot == std::string::npos) { return "application/octet-stream"; }

	std::string ext = path.substr(dot);
	if (ext == ".html") { return "text/html"; }
	if (ext == ".css") { return "text/css"; }
	if (ext == ".js") { return "application/javascript"; }
	if (ext == ".json") { return "application/json"; }
	if (ext == ".png") { return "image/png"; }
	return "application/octet-stream";
}

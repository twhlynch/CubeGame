#pragma once

#include <Rayne.h>

namespace CG
{

class LANServer
{
public:
	LANServer() = default;
	~LANServer() { Stop(); }

	void Update(float delta) {};

	bool Start();
	void Stop();
	[[nodiscard]] bool IsRunning() const { return _isRunning; }
	[[nodiscard]] uint16_t GetPort() const { return 8080; }
	[[nodiscard]] std::string GetAddressString() const { return "127.0.0.1"; }

private:
	bool _isRunning;
};

} // namespace CG

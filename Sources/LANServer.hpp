#pragma once

#include <Rayne.h>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "RNSceneNode.h"
#include "WebSocketServer.hpp"

namespace CG
{

class LANServer
{
public:
	LANServer() = default;
	~LANServer() { Stop(); }

	void Update(float delta);

	bool Start();
	void Stop();
	[[nodiscard]] bool IsRunning() const { return _server.IsRunning(); }
	[[nodiscard]] uint16_t GetPort() const { return _server.GetPort(); }
	[[nodiscard]] std::string GetAddressString() const { return _address; }

private:
	struct GroupState
	{
		uint32_t id;
		RN::Vector3 pos;
		RN::Quaternion rot;
		RN::Vector3 scale;
	};

	WebSocketServer _server;
	std::string _address;
	float _broadcastTimer = 0.0f;

	uint32_t _nextObjectId = 0;
	std::unordered_map<const RN::SceneNode *, GroupState> _lastSent;
};

} // namespace CG

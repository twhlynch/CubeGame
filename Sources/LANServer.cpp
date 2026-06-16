#include "LANServer.hpp"

#include <ixwebsocket/IXNetSystem.h>

#include <array>
#include <cmath>
#include <cstdio>

#include "PhysicsGroup.hpp"
#include "RNConstants.h"
#include "World.hpp"

namespace CG
{

bool LANServer::Start()
{
	if (!_server.Start(8080)) { return false; }

	_address = WebSocketServer::GetLocalIP();
	return true;
}

void LANServer::Stop()
{
	_server.Stop();
	_address.clear();
	_broadcastTimer = 0.0f;
	_nextObjectId = 0;
	_lastSent.clear();
}

void LANServer::Update(float delta)
{
	if (!_server.IsRunning()) { return; }

	World *world = World::GetSharedInstance();

	_broadcastTimer += delta;
	if (_broadcastTimer < 1.0f / 15.0f) { return; }
	_broadcastTimer = 0.0f;

	RN::Camera *camera = world->GetHeadCamera();
	if (!camera) { return; }

	RN::Vector3 playerPos = camera->GetWorldPosition();
	RN::Quaternion playerRot = camera->GetWorldRotation();

	std::string result;
	result.reserve(32768);

	std::array<char, 256> header;
	int len = std::snprintf(
		header.data(), header.size(),
		R"({"p":[%.3f,%.3f,%.3f],"r":[%.3f,%.3f,%.3f,%.3f],"s":[)",
		playerPos.x, playerPos.y, playerPos.z,
		playerRot.x, playerRot.y, playerRot.z, playerRot.w);
	result.append(header.data(), static_cast<size_t>(len));

	bool first = true;

	world->GetLevelNodes()->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t, bool &) {
		auto *group = dynamic_cast<PhysicsGroup *>(node);
		if (!group) { return; }

		RN::Vector3 pos = group->GetWorldPosition();
		RN::Quaternion rot = group->GetWorldRotation();
		RN::Vector3 scale = group->GetScale();
		auto *objects = group->GetObjects();
		if (objects->GetCount() > 0)
		{
			auto *obj = objects->GetObjectAtIndex<RN::Entity>(0);
			scale = obj->GetScale();
		}

		size_t sourceIndex = group->GetSourceIndex();

		auto it = _lastSent.find(node);
		uint32_t id;
		bool changed;

		if (it == _lastSent.end())
		{
			id = _nextObjectId++;
			it = _lastSent.emplace(node, GroupState{.id = id, .pos = pos, .rot = rot, .scale = scale}).first;
			changed = true;
		}
		else
		{
			id = it->second.id;
			changed = (pos - it->second.pos).GetLength() >= 0.001f ||
					  std::abs(rot.x - it->second.rot.x) >= 0.001f ||
					  std::abs(rot.y - it->second.rot.y) >= 0.001f ||
					  std::abs(rot.z - it->second.rot.z) >= 0.001f ||
					  std::abs(rot.w - it->second.rot.w) >= 0.001f ||
					  std::abs(scale.x - it->second.scale.x) >= RN::k::EpsilonFloat;
		}

		if (!first) { result += ','; }
		first = false;

		if (changed)
		{
			{
				std::array<char, 144> entry;
				int len = std::snprintf(
					entry.data(), entry.size(),
					"[%u,%zu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f]",
					id, sourceIndex,
					pos.x, pos.y, pos.z,
					rot.x, rot.y, rot.z, rot.w,
					scale.x, scale.y, scale.z);
				result.append(entry.data(), static_cast<size_t>(len));
			}

			it->second.pos = pos;
			it->second.rot = rot;
			it->second.scale = scale;
		}
		else
		{
			{
				std::array<char, 16> entry;
				int len = std::snprintf(entry.data(), entry.size(), "[%u]", id);
				result.append(entry.data(), static_cast<size_t>(len));
			}
		}
	});

	result += "]}";

	_server.Broadcast(result);
}

} // namespace CG

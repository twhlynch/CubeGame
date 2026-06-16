#include "LANServer.hpp"

#include <ixwebsocket/IXNetSystem.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

#include "Hand.hpp"
#include "PhysicsGroup.hpp"
#include "PhysicsObjects.hpp"
#include "RNConstants.h"
#include "World.hpp"

namespace CG
{

bool LANServer::Start()
{
	_server.SetOnConnect([this](uint32_t) {
		_lastSent.clear();
		_broadcastTimer = 1.0f / 15.0f;
	});

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
		header.data(), header.size(), R"({"p":[%.3f,%.3f,%.3f],"r":[%.3f,%.3f,%.3f,%.3f],"s":[)", playerPos.x, playerPos.y, playerPos.z, playerRot.x, playerRot.y, playerRot.z, playerRot.w
	);
	if (len > 0)
	{
		result.append(
			header.data(),
			std::min(static_cast<size_t>(len), header.size() - 1)
		);
	}

	bool first = true;

	world->GetLevelNodes()->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t, bool &) {
		auto *group = dynamic_cast<PhysicsGroup *>(node);
		if (!group) { return; }

		auto *objects = group->GetObjects();
		objects->Enumerate<PhysicsObject>([&](PhysicsObject *obj, size_t, bool &) {
			RN::Vector3 pos = obj->GetWorldPosition();
			RN::Quaternion rot = obj->GetWorldRotation();
			RN::Vector3 scale = obj->GetWorldScale();
			size_t sourceIndex = obj->GetSourceIndex();

			auto it = _lastSent.find(obj);
			uint32_t id;
			bool changed;

			if (it == _lastSent.end())
			{
				id = _nextObjectId++;
				it = _lastSent.emplace(obj, ObjectState {.id = id, .pos = pos, .rot = rot, .scale = scale}).first;
				changed = true;
			}
			else
			{
				id = it->second.id;
				changed =
					std::abs(pos.x - it->second.pos.x) >= 0.001f ||
					std::abs(pos.y - it->second.pos.y) >= 0.001f ||
					std::abs(pos.z - it->second.pos.z) >= 0.001f ||
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
					std::array<char, 256> entry;
					int len = std::snprintf(
						entry.data(), entry.size(), "[%u,%zu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f]", id, sourceIndex, pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, rot.w, scale.x, scale.y, scale.z
					);
					if (len > 0)
					{
						result.append(
							entry.data(),
							std::min(static_cast<size_t>(len), entry.size() - 1)
						);
					}
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
					if (len > 0)
					{
						result.append(
							entry.data(),
							std::min(static_cast<size_t>(len), entry.size() - 1)
						);
					}
				}
			}
		});
	});

	result += R"(],"h":[)";

	for (size_t handIndex = 0; handIndex < 2; handIndex++)
	{
		if (handIndex > 0) { result += ','; }
		result += '[';

		Hand *hand = world->GetHand(handIndex);
		if (hand)
		{
			const std::array<size_t, 5> tips = {Joint::ThumbTip, Joint::IndexTip, Joint::MiddleTip, Joint::RingTip, Joint::LittleTip};
			for (size_t t = 0; t < tips.size(); t++)
			{
				if (t > 0) { result += ','; }
				RN::Vector3 pos = hand->GetFingerTipPosition(tips[t]);
				std::array<char, 64> buf;
				int len = std::snprintf(buf.data(), buf.size(), "%.3f,%.3f,%.3f", pos.x, pos.y, pos.z);
				if (len > 0)
				{
					result.append(buf.data(), std::min(static_cast<size_t>(len), buf.size() - 1));
				}
			}
		}

		result += ']';
	}

	result += "]}";

	_server.Broadcast(result);
}

} // namespace CG

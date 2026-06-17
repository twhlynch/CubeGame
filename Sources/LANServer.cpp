#include "LANServer.hpp"

#include <ixwebsocket/IXNetSystem.h>

#include <Rayne.h>

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

	if (!_server.Start(8081)) { return false; }

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

	auto *jsonString = SerializeScene();
	if (jsonString)
	{
		_server.Broadcast(jsonString->GetUTF8String());
	}
}

RN::String *LANServer::SerializeScene()
{
	World *world = World::GetSharedInstance();

	RN::Camera *camera = world->GetHeadCamera();
	if (!camera) { return nullptr; }

	auto *root = new RN::Dictionary();

	// player
	RN::Vector3 playerPos = camera->GetWorldPosition();
	RN::Quaternion playerRot = camera->GetWorldRotation();

	auto *pos = new RN::Array();
	pos->AddObject(RN::Number::WithFloat(playerPos.x));
	pos->AddObject(RN::Number::WithFloat(playerPos.y));
	pos->AddObject(RN::Number::WithFloat(playerPos.z));
	root->SetValueForKey(pos->Autorelease(), "p");

	auto *rot = new RN::Array();
	rot->AddObject(RN::Number::WithFloat(playerRot.x));
	rot->AddObject(RN::Number::WithFloat(playerRot.y));
	rot->AddObject(RN::Number::WithFloat(playerRot.z));
	rot->AddObject(RN::Number::WithFloat(playerRot.w));
	root->SetValueForKey(rot->Autorelease(), "r");

	// shapes
	auto *objects = new RN::Array();

	world->GetLevelNodes()->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t, bool &) {
		auto *group = dynamic_cast<PhysicsGroup *>(node);
		if (!group) { return; }

		group->GetObjects()->Enumerate<PhysicsObject>([&](PhysicsObject *obj, size_t, bool &) {
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

			auto *entry = new RN::Array();
			entry->AddObject(RN::Number::WithUint32(id));

			if (changed)
			{
				entry->AddObject(RN::Number::WithUint64(sourceIndex));
				entry->AddObject(RN::Number::WithFloat(pos.x));
				entry->AddObject(RN::Number::WithFloat(pos.y));
				entry->AddObject(RN::Number::WithFloat(pos.z));
				entry->AddObject(RN::Number::WithFloat(rot.x));
				entry->AddObject(RN::Number::WithFloat(rot.y));
				entry->AddObject(RN::Number::WithFloat(rot.z));
				entry->AddObject(RN::Number::WithFloat(rot.w));
				entry->AddObject(RN::Number::WithFloat(scale.x));
				entry->AddObject(RN::Number::WithFloat(scale.y));
				entry->AddObject(RN::Number::WithFloat(scale.z));

				it->second.pos = pos;
				it->second.rot = rot;
				it->second.scale = scale;
			}

			objects->AddObject(entry->Autorelease());
		});
	});

	root->SetValueForKey(objects->Autorelease(), "s");

	// hands
	auto *hands = new RN::Array();

	for (size_t handIndex = 0; handIndex < 2; handIndex++)
	{
		auto *handArray = new RN::Array();

		Hand *hand = world->GetHand(handIndex);
		if (hand)
		{
			const std::array<size_t, 5> tips = {
				Joint::ThumbTip, Joint::IndexTip, Joint::MiddleTip, Joint::RingTip, Joint::LittleTip
			};
			for (auto tip : tips)
			{
				RN::Vector3 tipPos = hand->GetFingerTipPosition(tip);
				handArray->AddObject(RN::Number::WithFloat(tipPos.x));
				handArray->AddObject(RN::Number::WithFloat(tipPos.y));
				handArray->AddObject(RN::Number::WithFloat(tipPos.z));
			}
		}

		hands->AddObject(handArray->Autorelease());
	}

	root->SetValueForKey(hands->Autorelease(), "h");

	return RN::JSONSerialization::JSONStringFromObject(root->Autorelease());
}

} // namespace CG

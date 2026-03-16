#include "PartsPicker.hpp"

#include <Base/RNBase.h>
#include <Math/RNVector.h>
#include <Objects/RNArray.h>
#include <Scene/RNEntity.h>

#include "ObjectManager.hpp"
#include "Types.hpp"
#include "World.hpp"

namespace CG
{

static constexpr float scale = 0.01f;
static constexpr float gap = 0.04f;
static constexpr float handOffset = 0.08f;

PartsPicker::PartsPicker() : _objects(new RN::Array())
{
	World *world = World::GetSharedInstance();

	static auto *material = new RN::JoltMaterial();
	static auto *shape = new RN::JoltBoxShape(scale, material->Autorelease(), scale);

	constexpr float centerOffset = ((static_cast<float>(ObjectManager::GetColorCount()) / 2.0f) * gap) - (gap / 2.0f);

	for (size_t j = 0; j < ObjectManager::GetShapeCount(); ++j)
	{
		for (size_t i = 0; i < ObjectManager::GetColorCount(); ++i)
		{
			RN::Vector3 position(
				0,
				(static_cast<float>(j) * gap) + handOffset,
				(static_cast<float>(i) * gap) - centerOffset);

			const size_t index = (j * ObjectManager::GetColorCount()) + i;

			auto *part = world->GetObjectManager()->CreatePartWithIndex(index);
			part->SetScale(scale);
			part->SetPosition(position);
			part->SetIndex(index);

			auto *physicsBody = RN::JoltStaticBody::WithShape(shape);
			physicsBody->SetCollisionFilter(Types::CollisionPartPicker, Types::CollisionTest);
			part->AddAttachment(physicsBody);
			_objects->AddObject(part);

			AddChild(part);
		}
	}

	SetHidden(true);
}

PartsPicker::~PartsPicker()
{
	_objects->Autorelease();
}

PhysicsGroup *PartsPicker::CreatePhysicsObjectForPart(Part *part)
{
	World *world = World::GetSharedInstance();

	return world->GetObjectManager()->CreatePhysicsObjectWithIndex(part->GetIndex());
}

void PartsPicker::SetHidden(bool hidden)
{
	if (hidden == _isHidden) { return; }
	_isHidden = hidden;

	if (hidden)
	{
		AddFlags(RN::SceneNode::Flags::Hidden);
		_objects->Enumerate<Part>([&](Part *object, size_t, bool &) {
			object->AddFlags(RN::SceneNode::Flags::Hidden);
		});
	}
	else
	{
		RemoveFlags(RN::SceneNode::Flags::Hidden);
		_objects->Enumerate<Part>([&](Part *object, size_t, bool &) {
			object->RemoveFlags(RN::SceneNode::Flags::Hidden);
		});
	}
}

} // namespace CG

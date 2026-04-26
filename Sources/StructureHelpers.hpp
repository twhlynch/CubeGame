#include "ObjectManager.hpp"
#include "World.hpp"

namespace CG::Structures
{

static constexpr float structureScale = 4.0f;

// ring of towers
static void AddTowers()
{
	World *world = World::GetSharedInstance();
	ObjectManager *objectManager = world->GetObjectManager();
	RN::Camera *camera = world->GetHeadCamera();

	RN::Vector3 basePosition(0.0f);
	if (camera)
	{
		basePosition = camera->GetWorldPosition();
		basePosition.y = 0.0f;
	}

	const size_t segments = 8;
	for (size_t slice = 0; slice < segments; slice++)
	{
		const float frac = static_cast<float>(slice) / segments;
		const float phi = 2.0f * RN::k::Pi * frac;

		const float x = std::sin(phi);
		const float z = std::cos(phi);

		for (size_t y = 0; y < ObjectManager::GetColorCount(); y++)
		{
			auto *group = objectManager->CreatePhysicsObjectWithIndex(y);
			auto *object = group->GetObjects()->GetFirstObject<PhysicsObject>();
			if (!object) { continue; }

			group->SetScale(structureScale);
			group->CreatePhysicsBody();
			group->GetPhysicsBody()->SetMass(1);

			const float scale = object->GetWorldScale().x * structureScale * 0.5f;
			const float radius = scale * 0.5f;

			group->SetWorldPosition({
				basePosition.x + (x * 3.0f),
				basePosition.y + (scale * static_cast<float>(y)) + scale,
				basePosition.z + (z * 3.0f),
			});

			world->AddLevelNode(group->Autorelease());
		}
	}
}

// brick wall with cubes
static void AddWall()
{
	World *world = World::GetSharedInstance();
	ObjectManager *objectManager = world->GetObjectManager();
	RN::Camera *camera = world->GetHeadCamera();

	RN::Vector3 basePosition(0.0f, 0.0f, 1.0f);
	RN::Vector3 right(1.0f, 0.0f, 0.0f);
	RN::Vector3 forward(0.0f, 0.0f, 1.0f);
	RN::Quaternion rotation{};

	if (camera)
	{
		forward = camera->GetForward();
		forward.y = 0.0f;
		forward.Normalize();

		right = RN::Vector3(forward.z, 0.0f, -forward.x);

		rotation = RN::Quaternion::WithLookAt(-forward, RN::Vector3(0.0f, 1.0f, 0.0f));

		basePosition = camera->GetWorldPosition() + forward;
		basePosition.y = 0.0f;
	}

	for (size_t y = 0; y < 6; y++)
	{
		for (size_t x = 0; x < 6; x++)
		{
			const size_t objectIndex = x % ObjectManager::GetColorCount();

			auto *group = objectManager->CreatePhysicsObjectWithIndex(objectIndex);
			auto *object = group->GetObjects()->GetFirstObject<PhysicsCube>();
			if (!object) { continue; }

			group->SetScale(structureScale);
			group->CreatePhysicsBody();
			group->GetPhysicsBody()->SetMass(1);
			group->SetWorldRotation(rotation);

			const float scale = object->GetWorldScale().x * structureScale * 0.5f;
			const float radius = scale * 0.5f;
			const float halfWallWidth = scale * 2.5f;

			const float offsetX = (scale * static_cast<float>(x)) + (radius * static_cast<float>(y % 2)) - halfWallWidth;
			const float offsetY = (scale * static_cast<float>(y)) + scale;

			group->SetWorldPosition({
				basePosition.x + (right.x * offsetX),
				basePosition.y + offsetY,
				basePosition.z + (right.z * offsetX),
			});

			world->AddLevelNode(group->Autorelease());
		}
	}
}

} // namespace CG::Structures

#include <Math/RNMatrixQuaternion.h>
#include <Math/RNVector.h>
#include <Scene/RNSceneNode.h>

#include <RNJoltShape.h>
#include <RayneConfig.h>

#include "World.hpp"

namespace CG::Physics
{

template <class T>
T *TestOverlap(
	RN::JoltShape *shape,
	const RN::Vector3 &position,
	const RN::Quaternion &rotation,
	RN::uint32 mask,
	const RN::SceneNode *skip = nullptr)
{
	static_assert(std::is_base_of_v<RN::SceneNode, T>);

	World *world = World::GetSharedInstance();

	// check for overlaps with test collision group
	auto overlaps = world->GetPhysicsWorld()->CheckOverlap(
		shape, position, rotation, 1.0f, Types::CollisionTest, mask);

	// find an overlap derived from T
	for (const auto &info : overlaps)
	{
		if (!info.node) { continue; }

		auto *object = info.node->Downcast<T>();
		if (!object || object == skip) { continue; }

		return object;
	}

	return nullptr;
}

} // namespace CG::Physics

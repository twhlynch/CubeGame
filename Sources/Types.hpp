#pragma once

#include <Rayne.h>

namespace ART::Types
{
enum CollisionType
{
	CollisionNone = 0,
	CollisionLevel = 1 << 0,
	CollisionGrabbable = 1 << 1,
	CollisionPartPicker = 1 << 2,
	CollisionTest = 1 << 3,

	CollisionAll = 0xffffffff
};

enum class MaterialType : uint8_t
{
	MaterialDefault
};
} // namespace ART::Types

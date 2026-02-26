#pragma once

#include <Rayne.h>

namespace ART::Types
{
enum CollisionType
{
	CollisionLevel = 1 << 0,

	CollisionAll = 0xffffffff
};

enum class MaterialType : uint8_t
{
	MaterialDefault
};
} // namespace ART::Types

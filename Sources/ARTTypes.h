//
//  ARTTypes.h
//  AR-Test
//
//  Copyright 2026 by index. All rights reserved.
//

#ifndef __ART_TYPES_H_
#define __ART_TYPES_H_

#include "Rayne.h"

namespace ART
{
namespace Types
{
enum CollisionType
{
	CollisionLevel = 1 << 0,

	CollisionAll = 0xffffffff
};

enum MaterialType
{
	MaterialDefault
};
} // namespace Types
} // namespace ART

#endif /* defined(__ART_TYPES_H_) */

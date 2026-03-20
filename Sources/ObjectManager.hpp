#pragma once

#include <Objects/RNArray.h>

#include <RNJoltShape.h>

#include "Part.hpp"
#include "PhysicsGroup.hpp"

namespace CG
{

class ObjectManager
{
public:
	ObjectManager();
	~ObjectManager();

	Part *CreatePartWithIndex(size_t index);
	RN::Model *GetModelWithIndex(size_t index);
	PhysicsGroup *CreatePhysicsObjectWithIndex(size_t index);

	RN::Mesh *GetMeshWithIndex(size_t index);

	static constexpr size_t GetShapeCount() { return 3; }
	static constexpr size_t GetColorCount() { return 6; }

	std::array<RN::Mesh *, 3> _meshes;

private:
	RN::Array *_models;
};

} // namespace CG

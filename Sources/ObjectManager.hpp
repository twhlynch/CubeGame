#pragma once

#include <Objects/RNArray.h>

#include <RNJoltShape.h>

#include "Part.hpp"
#include "PhysicsGroup.hpp"

namespace CG
{

constexpr size_t shapeCount = 5;

class ObjectManager
{
public:
	ObjectManager();
	~ObjectManager();

	Part *CreatePartWithIndex(size_t index);
	RN::Model *GetModelWithIndex(size_t index);
	PhysicsGroup *CreatePhysicsObjectWithIndex(size_t index);

	RN::Mesh *GetMeshWithIndex(size_t index);
	RN::Mesh *GetPhysicsMeshWithIndex(size_t index);

	static constexpr size_t GetShapeCount() { return shapeCount; }
	static constexpr size_t GetColorCount() { return 9; }

private:
	RN::Array *_models;

	std::array<RN::Mesh *, shapeCount> _meshes;
	std::array<RN::Mesh *, shapeCount> _physicsMeshes;
};

} // namespace CG

#pragma once

#include <Objects/RNArray.h>
#include <Scene/RNSceneNode.h>

#include "Part.hpp"
#include "PhysicsGroup.hpp"

namespace CG
{

class PartsPicker : public RN::SceneNode
{
public:
	PartsPicker();
	~PartsPicker() override;

	void SetHidden(bool hidden);
	bool GetHidden() const { return _isHidden; }

	PhysicsGroup *CreatePhysicsObjectForPart(Part *part);

private:
	bool _isHidden;

	RN::Array *_objects;
};

} // namespace CG

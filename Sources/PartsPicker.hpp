#pragma once

#include <Rayne.h>

namespace CG
{

static constexpr size_t count = 6;

class PartsPicker : public RN::SceneNode
{
public:
	PartsPicker();

	void SetHidden(bool hidden);
	bool GetHidden() const { return _isHidden; }

private:
	bool _isHidden;

	std::array<RN::Entity *, count> _cubes;
};

} // namespace CG

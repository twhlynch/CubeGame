#include "Part.hpp"

namespace CG
{

RNDefineMeta(Part, RN::Entity);

Part::Part(RN::Model *model)
	: RN::Entity(model), _index(0)
{
}

void Part::SetIndex(size_t index)
{
	_index = index;
}

size_t Part::GetIndex() const
{
	return _index;
}

} // namespace CG

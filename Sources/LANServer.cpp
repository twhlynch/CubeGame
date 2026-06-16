#include "LANServer.hpp"

namespace CG
{

bool LANServer::Start()
{
	if (_isRunning)
	{
		return false;
	}

	_isRunning = true;
	return true;
}

void LANServer::Stop()
{
	_isRunning = false;
}

void LANServer::Update(float delta)
{
	if (!_isRunning)
	{
		return;
	}
}

} // namespace CG

#include "rhi\RHI.h"

#include "volk\volk.h"

namespace lux::rhi
{
	RHI::RHI()
		: isInitialized(false)
	{

	}

	RHI::~RHI()
	{

	}

	bool RHI::Initialize(const Window& window)
	{
		isInitialized = true;

		return true;
	}
}
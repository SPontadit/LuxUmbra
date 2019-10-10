#include "rhi\RHI.h"

#include "rhi\LuxVkImpl.h"

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
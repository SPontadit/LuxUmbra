#include "rhi\RHI.h"

#ifdef _WIN32

#define VK_USE_PLATFORM_WIN32_KHR

#endif // _WIN32

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
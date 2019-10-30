#include "resource\Texture.h"

namespace lux::resource
{

	using namespace lux;

	Texture::Texture() noexcept
		: sampler(VK_NULL_HANDLE)
	{

	}

	Texture::~Texture() noexcept
	{

	}

} // namespace lux::resource
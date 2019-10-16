#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include "Luxumbra.h"

#include "rhi\Image.h"

namespace lux::resource
{
	using namespace lux;

	class Texture
	{
	public:
		Texture() noexcept;
		Texture(const Texture&) = delete;
		Texture(Texture&&) = delete;

		~Texture() noexcept;

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;

		rhi::Image image;
	};

} // namespace lux::resource

#endif // TEXTURE_H_INCLUDED
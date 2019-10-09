#ifndef RHI_H_INCLUDED
#define RHI_H_INCLUDED

#include "Window.h"

namespace lux::rhi
{

	class RHI
	{
	public:
		RHI();
		RHI(const RHI&) = delete;
		RHI(RHI&&) = delete;

		~RHI();

		const RHI& operator=(const RHI&) = delete;
		const RHI& operator=(RHI&&) = delete;

		bool Initialize(const Window& window);

	private:
		bool isInitialized;
	};

} // namespace lux::rhi

#endif // RHI_H_INCLUDED
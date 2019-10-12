#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include "Luxumbra.h"

namespace lux::scene
{

	class Scene
	{
	public:
		Scene() noexcept;
		Scene(const Scene&) = delete;
		Scene(Scene&&) = delete;

		~Scene() noexcept = default;

		const Scene& operator=(const Scene&) = delete;
		const Scene& operator=(Scene&&) = delete;

	private:
	};

} // namespace lux::scene

#endif // SCENE_H_INCLUDED
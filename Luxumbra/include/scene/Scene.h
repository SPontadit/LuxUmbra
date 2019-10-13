#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "scene\Node.h"

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

		Node* AddNode(glm::vec3 position, glm::quat rotation, Node* parent, bool isWorldPosition = false) noexcept;

	private:
		std::vector<Node*> nodes;
	};

} // namespace lux::scene

#endif // SCENE_H_INCLUDED
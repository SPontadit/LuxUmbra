#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "Window.h"
#include "scene\Node.h"
#include "scene\CameraNode.h"

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

		bool Initialize(const Window& window) noexcept;

		Node* AddNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition) noexcept;
		CameraNode* AddCameraNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition, float fovy, float nearDist, float farDist) noexcept;

	private:
		bool isInititalized;

		const Window* window;

		std::vector<Node*> nodes;
	};

} // namespace lux::scene

#endif // SCENE_H_INCLUDED
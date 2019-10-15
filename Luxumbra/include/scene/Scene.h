#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "Window.h"
#include "resource\ResourceManager.h"
#include "scene\Node.h"
#include "scene\CameraNode.h"
#include "scene\MeshNode.h"
#include "scene\LightNode.h"

namespace lux::scene
{

	class Scene
	{
	public:
		Scene() noexcept;
		Scene(const Scene&) = delete;
		Scene(Scene&&) = delete;

		~Scene() noexcept;

		const Scene& operator=(const Scene&) = delete;
		const Scene& operator=(Scene&&) = delete;

		void Initialize(const Window& window, resource::ResourceManager& resourceManager) noexcept;

		CameraNode* GetCurrentCamera() const noexcept;
		const std::vector<MeshNode*>& GetMeshNodes() const noexcept;
		const std::vector<LightNode*>& GetLightNodes() const noexcept;

		Node* AddNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition) noexcept;
		CameraNode* AddCameraNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition, float fovy, float nearDist, float farDist, bool makeCurrentCamera) noexcept;
		MeshNode* AddMeshNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition, const std::string& meshFileName) noexcept;
		MeshNode* AddMeshNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition, resource::MeshPrimitive meshPrimitive) noexcept;
		LightNode* AddLightNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition, LightType type, glm::vec3 color) noexcept;

	private:
		const Window* window;
		resource::ResourceManager* resourceManager;

		std::vector<Node*> nodes;
		std::vector<MeshNode*> meshNodes;
		std::vector<LightNode*> lightNodes;
		CameraNode* currentCamera;
	};

} // namespace lux::scene

#endif // SCENE_H_INCLUDED
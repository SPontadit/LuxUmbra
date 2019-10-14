#include "scene\Scene.h"

namespace lux::scene
{
	Scene::Scene() noexcept
		: window(nullptr), resourceManager(nullptr), nodes(0), meshNodes(0), currentCamera(nullptr)
	{

	}

	void Scene::Initialize(const Window& window, resource::ResourceManager& resourceManager) noexcept
	{
		this->window = &window;
		this->resourceManager = &resourceManager;

		nodes.reserve(128);
		meshNodes.reserve(128);
	}

	CameraNode* Scene::GetCurrentCamera() const noexcept
	{
		return currentCamera;
	}

	const std::vector<MeshNode*>& Scene::GetMeshNodes() const noexcept
	{
		return meshNodes;
	}

	Node* Scene::AddNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition) noexcept
	{
		Node* node;
		
		if (isWorldPosition)
		{
			node = new Node(parent);
			node->SetWorldPosition(position);
			node->SetWorldRotation(rotation);
		}
		else
			node = new Node(parent, position, rotation);

		nodes.push_back(node);

		return node;
	}

	CameraNode* Scene::AddCameraNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition, float fovy, float nearDist, float farDist, bool makeCurrentCamera) noexcept
	{
		CameraNode* cameraNode;

		if (isWorldPosition)
		{
			cameraNode = new CameraNode(parent, window, fovy, nearDist, farDist);
			cameraNode->SetWorldPosition(position);
			cameraNode->SetWorldRotation(rotation);
		}
		else
			cameraNode = new CameraNode(parent, position, rotation, window, fovy, nearDist, farDist);

		nodes.push_back(cameraNode);

		if (makeCurrentCamera)
			currentCamera = cameraNode;

		return cameraNode;
	}

	MeshNode* Scene::AddMeshNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition, const std::string& meshFileName) noexcept
	{
		MeshNode* meshNode;

		if (isWorldPosition)
		{
			meshNode = new MeshNode(parent, resourceManager->GetMesh(meshFileName));
			meshNode->SetWorldPosition(position);
			meshNode->SetWorldRotation(rotation);
		}
		else
			meshNode = new MeshNode(parent, position, rotation, resourceManager->GetMesh(meshFileName));

		nodes.push_back(meshNode);
		meshNodes.push_back(meshNode);

		return meshNode;
	}

	MeshNode* Scene::AddMeshNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition, resource::MeshPrimitive meshPrimitive) noexcept
	{
		MeshNode* meshNode;

		if (isWorldPosition)
		{
			meshNode = new MeshNode(parent, resourceManager->GetMesh(meshPrimitive));
			meshNode->SetWorldPosition(position);
			meshNode->SetWorldRotation(rotation);
		}
		else
			meshNode = new MeshNode(parent, position, rotation, resourceManager->GetMesh(meshPrimitive));

		nodes.push_back(meshNode);
		meshNodes.push_back(meshNode);

		return meshNode;
	}

} // namespace lux::scene
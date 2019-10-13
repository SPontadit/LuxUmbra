#include "scene\Scene.h"

namespace lux::scene
{
	Scene::Scene() noexcept
		: window(nullptr), nodes(0)
	{

	}

	bool Scene::Initialize(const Window& window) noexcept
	{
		this->window = &window;

		nodes.reserve(128);

		return true;
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

	CameraNode* Scene::AddCameraNode(Node* parent, glm::vec3 position, glm::quat rotation, bool isWorldPosition, float fovy, float nearDist, float farDist) noexcept
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

		return cameraNode;
	}

} // namespace lux::scene
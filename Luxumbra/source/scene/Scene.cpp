#include "scene\Scene.h"

namespace lux::scene
{
	Scene::Scene() noexcept
		: nodes(0)
	{
		nodes.reserve(100);
	}

	Node* Scene::AddNode(glm::vec3 position, glm::quat rotation, Node* parent, bool isWorldPosition) noexcept
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

} // neamspace lux::scene
#include "scene\LightNode.h"

namespace lux::scene
{
	using namespace lux;



	LightNode::LightNode(Node* parent, LightType type) noexcept
		: Node(parent), type(type), color(1.0f)
	{

	}

	LightNode::LightNode(Node* parent, glm::vec3 position, glm::quat rotation, LightType type) noexcept
		: Node(parent, position, rotation), type(type), color(1.0f)
	{

	}

	void LightNode::SetColor(glm::vec3 newColor) noexcept
	{
		color = newColor;
	}

	glm::vec3 LightNode::GetColor() const noexcept
	{
		return color;
	}

} // namespace lux::scene
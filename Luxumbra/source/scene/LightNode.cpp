#include "scene\LightNode.h"

namespace lux::scene
{
	using namespace lux;

	LightNode::LightNode(Node* parent) noexcept
		: Node(parent), color(1.0f)
	{

	}

	LightNode::LightNode(Node* parent, glm::vec3 position, glm::quat rotation) noexcept
		: Node(parent, position, rotation), color(1.0f)
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
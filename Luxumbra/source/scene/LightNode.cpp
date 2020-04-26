#include "scene\LightNode.h"

namespace lux::scene
{
	using namespace lux;

	LightNode::LightNode(Node* parent, LightType type, glm::vec3 color, int16_t shadowMappingResourceIndex) noexcept
		: Node(parent), type(type), color(color), radius(1.f), shadowMappingResourceIndex(shadowMappingResourceIndex)
	{

	}

	LightNode::LightNode(Node* parent, glm::vec3 position, glm::vec3 rotation, LightType type, glm::vec3 color, int16_t shadowMappingResourceIndex) noexcept
		: Node(parent, position, rotation), type(type), color(color), radius(1.f), shadowMappingResourceIndex(shadowMappingResourceIndex)
	{

	}

	LightType LightNode::GetType() const noexcept
	{
		return type;
	}

	glm::vec3 LightNode::GetColor() const noexcept
	{
		return color;
	}

	void LightNode::SetColor(glm::vec3 newColor) noexcept
	{
		color = newColor;
	}

	float LightNode::GetRadius() const noexcept
	{
		return radius;
	}

	void LightNode::SetRadius(float newRadius) noexcept
	{
		radius = newRadius;
	}

	int16_t LightNode::GetShadowMappingResourceIndex() const noexcept
	{
		return shadowMappingResourceIndex;
	}

} // namespace lux::scene

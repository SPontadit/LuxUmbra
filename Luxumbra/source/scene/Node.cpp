#include "scene\Node.h"

#include "glm\gtc\matrix_transform.hpp"

namespace lux::scene
{
	Node::Node() noexcept
		: parent(nullptr), position(0.f), rotation(glm::identity<glm::quat>())
	{
	}

	glm::vec3 Node::GetLocalPosition() const noexcept
	{
		return position;
	}

	glm::quat Node::GetLocalRotation() const noexcept
	{
		return rotation;
	}

	glm::mat4 Node::GetLocalTransform() const noexcept
	{
		
		return glm::translate(glm::toMat4(rotation), position);
	}

	glm::vec3 Node::GetWorldPosition() const noexcept
	{
		glm::vec4 position = glm::vec4(GetLocalPosition(), 1.f);

		if (parent)
			position = parent->GetWorldTransform() * position;

		return position.xyz;
	}

	glm::quat Node::GetWorldRotation() const noexcept
	{
		glm::quat rotation = GetLocalRotation();

		if (parent)
			rotation = parent->GetWorldRotation() * rotation;

		return rotation;
	}

	glm::mat4 Node::GetWorldTransform() const noexcept
	{
		glm::mat4 transform = GetLocalTransform();

		if (parent)
			transform = parent->GetWorldTransform() * transform;

		return transform;
	}
}
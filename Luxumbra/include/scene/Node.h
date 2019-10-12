#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include "Luxumbra.h"

#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"

namespace lux::scene
{

	class Node
	{
	public:
		Node() noexcept;
		Node(const Node&) = delete;
		Node(Node&&) = delete;

		~Node() = default;

		const Node& operator=(const Node&) = delete;
		const Node& operator=(Node&&) = delete;

		glm::vec3 GetLocalPosition() const noexcept;
		glm::quat GetLocalRotation() const noexcept;
		glm::mat4 GetLocalTransform() const noexcept;

		glm::vec3 GetWorldPosition() const noexcept;
		glm::quat GetWorldRotation() const noexcept;
		glm::mat4 GetWorldTransform() const noexcept;

	private:
		Node* parent;

		glm::vec3 position;
		glm::quat rotation;
	};

} // namespace lux::scene

#endif // NODE_H_INCLUDED
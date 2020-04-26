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
		Node(Node* parent) noexcept;
		Node(Node* parent, glm::vec3 position, glm::vec3 rotation) noexcept;
		Node(const Node&) = delete;
		Node(Node&&) = delete;
			
		virtual ~Node() noexcept = default;

		const Node& operator=(const Node&) = delete;
		const Node& operator=(Node&&) = delete;

		glm::vec3 GetLocalPosition() const noexcept;
		glm::vec3 GetLocalRotation() const noexcept;
		glm::vec3 GetLocalScale() const noexcept;
		glm::mat4 GetLocalTransform() const noexcept;

		glm::vec3 GetWorldPosition() const noexcept;
		glm::quat GetWorldRotation() const noexcept;
		glm::mat4 GetWorldTransform() const noexcept;

		void SetLocalPosition(glm::vec3 newPosition) noexcept;
		void SetLocalRotation(glm::vec3 newRotation) noexcept;
		void SetLocalScale(glm::vec3 newScale) noexcept;

		void SetWorldPosition(glm::vec3 newPosition) noexcept;
		void SetWorldRotation(glm::quat newRotation) noexcept;

	private:
		Node* parent;

		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
	};

} // namespace lux::scene

#endif // NODE_H_INCLUDED
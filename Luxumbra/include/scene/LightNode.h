#ifndef LIGHT_NODE_H_INCLUDED
#define LIGHT_NODE_H_INCLUDED

#include "Luxumbra.h"

#include "scene\Node.h"

namespace lux::scene
{
	using namespace lux;

	class LightNode : public Node
	{
	public:
		LightNode() = delete;
		LightNode(Node* parent) noexcept;
		LightNode(Node* parent, glm::vec3 position, glm::quat rotation) noexcept;
		LightNode(const LightNode&) = delete;
		LightNode(LightNode&&) = delete;

		~LightNode() noexcept = default;

		const LightNode& operator=(const LightNode&) = delete;
		const LightNode& operator=(LightNode&&) = delete;

		void SetColor(glm::vec3 newColor) noexcept;
		glm::vec3 GetColor() const noexcept;

	private:
		glm::vec3 color;
	};

} // namespace lux::scene

#endif // LIGHT_NODE_H_INCLUDED
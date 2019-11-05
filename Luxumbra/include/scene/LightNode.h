#ifndef LIGHT_NODE_H_INCLUDED
#define LIGHT_NODE_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "scene\Node.h"
#include "scene\MeshNode.h"
#include "rhi\Image.h"

namespace lux::scene
{
	using namespace lux;

	enum class LightType : uint32_t
	{
		LIGHT_TYPE_DIRECTIONAL = 0,
		LIGHT_TYPE_POINT
	};

	class LightNode : public Node
	{
	public:
		LightNode() = delete;
		LightNode(Node* parent, LightType type, glm::vec3 color, int16_t shadowMappingResourceIndex) noexcept;
		LightNode(Node* parent, glm::vec3 position, glm::vec3 rotation, LightType type, glm::vec3 color, int16_t shadowMappingResourceIndex) noexcept;
		LightNode(const LightNode&) = delete;
		LightNode(LightNode&&) = delete;

		~LightNode() noexcept = default;

		const LightNode& operator=(const LightNode&) = delete;
		const LightNode& operator=(LightNode&&) = delete;

		LightType GetType() const noexcept;

		void SetColor(glm::vec3 newColor) noexcept;
		glm::vec3 GetColor() const noexcept;

		int16_t GetShadowMappingResourceIndex() const noexcept;

	private:
		LightType type;
		glm::vec3 color;

		int16_t shadowMappingResourceIndex;
	};

} // namespace lux::scene

#endif // LIGHT_NODE_H_INCLUDED
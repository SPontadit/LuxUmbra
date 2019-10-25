#ifndef LIGHT_NODE_H_INCLUDED
#define LIGHT_NODE_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "scene\Node.h"
#include "scene\MeshNode.h"

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
		LightNode(Node* parent, LightType type, glm::vec3 color) noexcept;
		LightNode(Node* parent, glm::vec3 position, glm::vec3 rotation, LightType type, glm::vec3 color) noexcept;
		LightNode(const LightNode&) = delete;
		LightNode(LightNode&&) = delete;

		~LightNode() noexcept = default;

		const LightNode& operator=(const LightNode&) = delete;
		const LightNode& operator=(LightNode&&) = delete;

		LightType GetType() const noexcept;

		void SetColor(glm::vec3 newColor) noexcept;
		glm::vec3 GetColor() const noexcept;

		glm::mat4 GetViewTransform() const noexcept;
		glm::mat4 GetProjectionTransform() const noexcept;

		void ComputeVolumeInfo(const std::vector<MeshNode*>& meshNodes) noexcept;

	private:
		LightType type;
		glm::vec3 color;

		union VolumeInfo
		{
			VolumeInfo(LightType type) noexcept;

			struct Directional
			{
				Directional() noexcept;

				glm::vec3 viewPos;
				float left, right, top, bottom;
				float nearDist, farDist;
			} directional;

			struct Point
			{
				Point() noexcept;

				float radius, attenuation;
			} point;

		} volumeInfo;

		void ComputeDirectionalVolumeInfo(const std::vector<MeshNode*>& meshNodes) noexcept;
	};

} // namespace lux::scene

#endif // LIGHT_NODE_H_INCLUDED
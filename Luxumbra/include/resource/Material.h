#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "glm\glm.hpp"

#include "rhi\Buffer.h"

namespace lux::resource
{
	using namespace lux;

	struct MaterialParameters
	{
		// TODO: Add texture albedo, metallic, normal
		// Texture albedo
		// Texture metalness
		// Texture normal
		glm::vec3 baseColor;
		bool metallic;
		float perceptualRoughness;
		float reflectance;
	};

	struct GLSLMaterial
	{
		glm::vec4 color;
		glm::vec4 parameter;
	};

	class Material
	{
	public:
		Material(MaterialParameters parameter) noexcept;
		Material(const Material&) = delete;
		Material(Material&&) = delete;

		~Material() noexcept = default;

		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) = delete;
	

		MaterialParameters parameter;
		
		std::vector<rhi::Buffer> buffer;
		std::vector<VkDescriptorSet> descriptorSet;
	};

} // namespace lux::resource

#endif //MATERIAL_H_INCLUDED
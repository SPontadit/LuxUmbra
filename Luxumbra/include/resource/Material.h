#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED

#include "Luxumbra.h"

#include <vector>

#include "glm\glm.hpp"

#include "rhi\Buffer.h"
#include "resource\Texture.h"

namespace lux::resource
{
	using namespace lux;

	struct MaterialCreateInfo
	{
		std::shared_ptr<Texture> albedo;
		std::shared_ptr<Texture> normal;
		glm::vec3 baseColor;
		float metallic;
		float perceptualRoughness;
		float reflectance;
		bool isTransparent;
	};

	struct MaterialParameters
	{
		glm::vec3 baseColor;
		float metallic;
		float perceptualRoughness;
		float reflectance;
	};


	class Material
	{
	public:
		Material(const std::string& name, MaterialCreateInfo materialCI) noexcept;
		Material(const Material&) = delete;
		Material(Material&&) = delete;

		~Material() noexcept = default;

		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) = delete;
	

		// TODO: Add texture metallic
		// Texture metalness
		std::string name;

		bool isTransparent;
		MaterialParameters parameter;
		std::shared_ptr<Texture> albedo;
		std::shared_ptr<Texture> normal;

		
		std::vector<rhi::Buffer> buffer;
		std::vector<VkDescriptorSet> descriptorSet;
	};

} // namespace lux::resource

#endif //MATERIAL_H_INCLUDED
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

	enum TextureMask
	{
		ROUGHNESS_TEXTURE_MASK = 1,
		METALLIC_TEXTURE_MASK = 2
	};

	struct MaterialCreateInfo
	{
		std::shared_ptr<Texture> albedo;
		std::shared_ptr<Texture> normal;
		std::shared_ptr<Texture> metallicRoughness;
		std::shared_ptr<Texture> ambientOcclusion;
		glm::vec3 baseColor;
		float metallic;
		float perceptualRoughness;
		float reflectance;
		float clearCoat = 0.0f;
		float clearCoatPerceptualRoughness = 0.045f;
		bool isTransparent;
		int textureMask;
	};

	struct MaterialParameters
	{
		glm::vec3 baseColor;
		float metallic;
		float perceptualRoughness;
		float reflectance;
		float clearCoat;
		float clearCoatPerceptualRoughness;
		int textureMask;
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
		std::shared_ptr<Texture> metallicRoughness;
		std::shared_ptr<Texture> ambientOcclusion;
		
		std::vector<rhi::Buffer> buffer;
		std::vector<VkDescriptorSet> descriptorSet;
	};

} // namespace lux::resource

#endif //MATERIAL_H_INCLUDED
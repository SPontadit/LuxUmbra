#include "resource\Material.h"

namespace lux::resource
{

	Material::Material(const std::string& name, MaterialCreateInfo materialCI) noexcept
		: name(name), isTransparent(materialCI.isTransparent), albedo(materialCI.albedo), normal(materialCI.normal)
	{
		parameter.baseColor = materialCI.baseColor;
		parameter.reflectance = materialCI.reflectance;
		parameter.perceptualRoughness = materialCI.perceptualRoughness;
		parameter.metallic = materialCI.metallic;
	}

} // namespace lux::resource
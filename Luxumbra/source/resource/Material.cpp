#include "resource\Material.h"

namespace lux::resource
{

	Material::Material(const std::string& name, MaterialParameters parameter) noexcept
		: name(name), parameter(parameter)
	{

	}

} // namespace lux::resource
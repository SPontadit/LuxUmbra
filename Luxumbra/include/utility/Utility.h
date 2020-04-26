#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include "Luxumbra.h"

#include <string>
#include <vector>

namespace lux::utility
{

	std::vector<char> ReadFile(const std::string& filePath) noexcept;
	float Lerp(float a, float b, float t) noexcept;

} // namespace lux::utility

#endif // UTILITY_H_INCLUDED
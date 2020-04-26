#include "utility\Utility.h"

#include <fstream>

namespace lux::utility
{

	std::vector<char> ReadFile(const std::string& filePath) noexcept
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	float Lerp(float a, float b, float t) noexcept
	{
		return a + t * (b - a);
	}

} // namespace lux::utility
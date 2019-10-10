#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <string>
#include <vector>

namespace lux::utility
{

	std::vector<char> ReadFile(std::string filePath);

} // namespace lux::utility

#endif // UTILITY_H_INCLUDED
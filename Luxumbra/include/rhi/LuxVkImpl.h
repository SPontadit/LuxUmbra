#ifndef LUX_VK_IMPL_H_INCLUDED
#define LUX_VK_IMPL_H_INCLUDED

#include "Luxumbra.h"

#include "volk\volk.h"

#ifdef _DEBUG
#include "Logger.h"
#define CHECK_VK(vkFunctionCall) if (VK_SUCCESS != (vkFunctionCall)) { Logger::Log(LogLevel::LOG_LEVEL_ERROR, #vkFunctionCall); __debugbreak(); }
#define VULKAN_ENABLE_VALIDATION
#else // !DEBUG
#define CHECK_VK(vkFunctionCall) vkFunctionCall
#endif // DEBUG

#endif // LUX_VK_IMPL_H_INCLUDED
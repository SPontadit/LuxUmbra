#ifndef LUXUMBRA_H_INCLUDED
#define LUXUMBRA_H_INCLUDED

#include <stdint.h>

#include <memory>

#define TO_SIZE_T(x) static_cast<size_t>(x)
#define TO_UINT32_T(x) static_cast<uint32_t>(x)
#define TO_FLOAT(x) static_cast<float>(x)

#ifdef _DEBUG
#include <cassert>
#define ASSERT(x) if(x == false) { __debugbreak(); assert(false); }
#else // !_DEBUG
#define ASSERT(x)
#endif // _DEBUG

#define GLM_FORCE_SWIZZLE

#endif // LUXUMBRA_H_INCLUDED
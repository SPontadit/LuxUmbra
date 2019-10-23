#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

#include "Luxumbra.h"

#include "rhi\Buffer.h"
#include "AABB.h"

namespace lux::resource
{

	class Mesh
	{
	public:
		Mesh() noexcept;
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) = delete;

		~Mesh() noexcept;

		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) = delete;
	
		uint32_t indexCount;
		lux::rhi::Buffer vertexBuffer;
		lux::rhi::Buffer indexBuffer;

		AABB aabb;
	};

} // namespace lux::resource

#endif // MESH_H_INCLUDED
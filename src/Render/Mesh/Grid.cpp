#include "Mesh/Grid.h"

#include "Core/SContainer.hpp"

namespace sh::render
{
	SH_RENDER_API Grid::Grid()
	{
		core::SVector<Vertex> verts{};
		core::SVector<uint32_t> indices{};
		for (int w = -10; w < 11; ++w)
		{
			verts.push_back(Vertex{ glm::vec3{ static_cast<float>(w), 0.f, -10.f } });
			verts.push_back(Vertex{ glm::vec3{ static_cast<float>(w), 0.f, 10.f } });
			indices.push_back(indices.size());
			indices.push_back(indices.size());
		}
		for (int h = -10; h < 11; ++h)
		{
			verts.push_back(Vertex{ glm::vec3{ -10.f, 0.f, static_cast<float>(h) } });
			verts.push_back(Vertex{ glm::vec3{ 10.f, 0.f, static_cast<float>(h) } });
			indices.push_back(indices.size());
			indices.push_back(indices.size());
		}
		
		this->SetVertex(std::move(verts));
		this->SetIndices(std::move(indices));
		
		this->SetTopology(Mesh::Topology::Line);
	}
}//namespace
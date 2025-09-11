#include "Mesh/CapsuleMesh.h"

#include <glm/gtc/constants.hpp>
namespace sh::render
{
	SH_RENDER_API CapsuleMesh::CapsuleMesh(float radius, float height, int slices, int stacks)
	{
		std::vector<Vertex> verts{};
		std::vector<uint32_t> indices{};

		CreateInfo ci{};
		ci.radius = radius;
		ci.height = height;
		ci.stacks = stacks;
		ci.slices = slices;
		ci.yOffset = height / 2.0f;
		ci.bTop = true;

		CreateHemisphere(verts, indices, ci);
		ci.yOffset = -height / 2.0f;
		ci.bTop = false;
		CreateHemisphere(verts, indices, ci);
		ci.height = height;
		CreateCylinder(verts, indices, ci);
		CalculateTangents();

		this->SetVertex(std::move(verts));
		this->SetIndices(std::move(indices));

		this->SetTopology(Mesh::Topology::Face);
	}
	void CapsuleMesh::CreateHemisphere(std::vector<Vertex>& verts, std::vector<uint32_t>& indices, const CreateInfo& ci)
	{
		float h = ci.height;
		float radius = ci.radius;
		float yOffset = ci.yOffset;
		int stacks = ci.stacks;
		int slices = ci.slices;
		bool bTop = ci.bTop;
		for (int i = 0; i <= stacks; ++i) 
		{
			float theta = (float)i / stacks * (glm::pi<float>() / 2.0f);
			if (!bTop)
				theta += glm::pi<float>() / 2.0f; // 하단 반구

			float sinTheta = std::sin(theta);
			float cosTheta = std::cos(theta);

			for (int j = 0; j <= slices; ++j) 
			{
				float phi = (float)j / slices * 2.0f * glm::pi<float>();
				float sinPhi = std::sin(phi);
				float cosPhi = std::cos(phi);

				glm::vec3 pos
				{
					radius * sinTheta * cosPhi,
					radius * cosTheta + yOffset,
					radius * sinTheta * sinPhi
				};
				glm::vec3 normal = glm::normalize(pos - glm::vec3(0.0f, yOffset, 0.0f)); // 반구의 중심에서 버텍스로 향하는 벡터
				glm::vec2 uv{ (float)j / slices, (float)i / stacks };

				// 탄젠트 계산은 나중에
				Vertex vert{};
				vert.vertex = pos;
				vert.uv = uv;
				vert.normal = normal;
				verts.push_back(vert);
			}
		}

		int base = (int)verts.size() - (stacks + 1) * (slices + 1);
		for (int i = 0; i < stacks; ++i) 
		{
			for (int j = 0; j < slices; ++j) 
			{
				int a = base + i * (slices + 1) + j;
				int b = base + (i + 1) * (slices + 1) + j;
				int c = base + (i + 1) * (slices + 1) + (j + 1);
				int d = base + i * (slices + 1) + (j + 1);

				indices.push_back(a); indices.push_back(b); indices.push_back(d);
				indices.push_back(b); indices.push_back(c); indices.push_back(d);
			}
		}
	}
	void CapsuleMesh::CreateCylinder(std::vector<Vertex>& verts, std::vector<uint32_t>& indices, const CreateInfo& ci)
	{
		float h = ci.height;
		float radius = ci.radius;
		float yOffset = ci.yOffset;
		int stacks = ci.stacks;
		int slices = ci.slices;
		for (int i = 0; i <= 1; ++i) 
		{
			float y = i * h + yOffset;
			for (int j = 0; j <= slices; ++j) 
			{
				float phi = (float)j / slices * 2.0f * glm::pi<float>();
				float x = radius * std::cos(phi);
				float z = radius * std::sin(phi);

				glm::vec3 pos{ x, y, z };
				glm::vec3 normal{ x, 0.0f, z };
				normal = glm::normalize(normal);
				glm::vec2 uv{ (float)j / slices, i };

				// 탄젠트 계산은 나중에
				Vertex vert{};
				vert.vertex = pos;
				vert.uv = uv;
				vert.normal = normal;
				verts.push_back(vert);
			}
		}

		int base = (int)verts.size() - 2 * (slices + 1);
		for (int j = 0; j < slices; ++j) 
		{
			int a = base + j;
			int b = base + j + slices + 1;
			int c = base + j + slices + 2;
			int d = base + j + 1;

			indices.push_back(a); indices.push_back(b); indices.push_back(d);
			indices.push_back(b); indices.push_back(c); indices.push_back(d);
		}
	}
}//namespace
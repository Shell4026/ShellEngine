#pragma once
#include "../Mesh.h"

#include <vector>
namespace sh::render
{
	class CapsuleMesh : public Mesh
	{
	public:
		SH_RENDER_API CapsuleMesh(float radius, float height, int slices = 16, int stacks = 8);
	private:
		struct CreateInfo
		{
			float height;
			float radius;
			float yOffset;
			int slices;
			int stacks;
			bool bTop;
		};
		/// @brief 반구 생성
		/// @param bTop 상단 반구이면 true, 하단 반구이면 false
		/// @param yOffset y축 위치 오프셋
		void CreateHemisphere(std::vector<Vertex>& verts, std::vector<uint32_t>& indices, const CreateInfo& ci);
		void CreateCylinder(std::vector<Vertex>& verts, std::vector<uint32_t>& indices, const CreateInfo& ci);
	};
}//namespace
#include "Mesh/Plane.h"

namespace sh::render
{
	Plane::Plane()
	{
		this->SetVertex(
			{
				Mesh::Vertex{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f} },
				Mesh::Vertex{ {-0.5f, 0.5f,  0.0f}, {0.0f, 1.0f} },
				Mesh::Vertex{ {0.5f, 0.5f,  0.0f}, {1.0f, 1.0f} },
				Mesh::Vertex{ {0.5f, -0.5f,  0.0f}, {1.0f, 0.0f} }
			}
		);
		this->SetIndices(
			{
				0, 1, 2, 2, 3, 0
			}
		);
	}
}//namespace
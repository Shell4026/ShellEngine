#include "pch.h"
#include "Mesh/Plane.h"

namespace sh::render
{
	Plane::Plane()
	{
		this->SetVertex(
			{
				{-0.5f, 0.0f, -0.5f},
				{-0.5f, 0.0f, 0.5f},
				{0.5f, 0.0f, 0.5f},
				{0.5f, 0.0f, -0.5f}
			}
		);
		this->SetIndices(
			{
				0, 1, 2, 2, 3, 0
			}
		);
		this->SetUV( 
			{
				{0.0f, 0.0f},
				{0.0f, 1.0f},
				{1.0f, 1.0f},
				{1.0f, 0.0f}
			}
		);
	}
}//namespace
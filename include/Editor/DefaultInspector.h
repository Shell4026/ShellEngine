#include "CustomInspector.h"

#include "Render/Material.h"

namespace sh::editor
{
	class MaterialInspector : public CustomInspector<MaterialInspector, render::Material>
	{
	public:
		void RenderUI(void* obj) override;
	};
}//namespace
#include "Component/Render/TextRenderer.h"
#include "GameObject.h"

#include "Core/Util.h"
namespace sh::game
{
    TextRenderer::TextRenderer(GameObject& owner) :
        MeshRenderer(owner)
    {
        auto mat = static_cast<render::Material*>(core::SObject::GetSObjectUsingResolver(core::UUID{"bbc4ef7ec45dce223297a224f8093f23"})); // UITextMat
        assert(mat != nullptr);
        SetMaterial(mat);
    }
    SH_GAME_API void TextRenderer::Start()
    {
        if (GetMesh() == nullptr)
            Setup();
    }
    SH_GAME_API void TextRenderer::OnPropertyChanged(const core::reflection::Property& prop)
    {
        Super::OnPropertyChanged(prop);
        if (prop.GetName() == core::Util::ConstexprHash("txt"))
            Setup();
        else if (prop.GetName() == core::Util::ConstexprHash("font"))
            Setup();
    }
    SH_GAME_API void TextRenderer::SetText(const std::string& text)
    {
        txt = text;
        Setup();
    }
    SH_GAME_API void TextRenderer::SetText(std::string&& text)
    {
        txt = std::move(text);
        Setup();
    }
    void TextRenderer::Setup()
    {
        if (!core::IsValid(font))
            return;

        if (txt.empty())
            SetMesh(nullptr);
        else
        {
            render::Mesh* const mesh = CreateQuad();
            if (mesh->GetVertexCount() == 0)
                SetMesh(nullptr);
            else
                SetMesh(CreateQuad());
        }

        if (GetMaterial() != nullptr)
        {
            auto propBlock = GetMaterialPropertyBlock();
            if (propBlock == nullptr)
                SetMaterialPropertyBlock(std::make_unique<render::MaterialPropertyBlock>());
            propBlock = GetMaterialPropertyBlock();

            propBlock->SetProperty("tex", font->GetAtlases()[0]);
            UpdatePropertyBlockData();
        }
    }
    auto TextRenderer::CreateQuad() -> render::Mesh*
    {
        if (!core::IsValid(font))
            return nullptr;

        const char* start = txt.data();
        const char* end = start + txt.size();

        float x, y;
        x = y = 0.f;
        uint32_t prevUnicode = 0;

        const float lineHeight = font->GetLineHeightPx();

        render::Mesh* mesh = core::SObject::Create<render::Mesh>();

        std::vector<render::Mesh::Vertex> verts;
        std::vector<uint32_t> indices;
        while (start < end)
        {
            uint32_t unicode = 0;
            start = core::Util::UTF8ToUnicode(start, end, unicode);

            if (unicode == '\r') 
                continue;
            if (unicode == '\n')
            {
                x = 0.f;
                y += lineHeight;
                prevUnicode = 0;
                continue;
            }

            auto glyphPtr = font->GetGlyph(unicode);
            if (glyphPtr == nullptr)
                continue;

            float x0 = x + glyphPtr->bearingX;
            float y0 = y - glyphPtr->bearingY;
            float x1 = x0 + glyphPtr->w;
            float y1 = y0 - glyphPtr->h;

            uint32_t idx = verts.size();
            render::Mesh::Vertex vert{};
            vert.vertex = { x0, y0, 0.f };
            vert.uv = { glyphPtr->u0, glyphPtr->v0 };
            vert.normal = { 0.f, 0.f, 1.f };
            verts.push_back(vert);

            vert.vertex = { x1, y0, 0.f };
            vert.uv = { glyphPtr->u1, glyphPtr->v0 };
            vert.normal = { 0.f, 0.f, 1.f };
            verts.push_back(vert);

            vert.vertex = { x1, y1, 0.f };
            vert.uv = { glyphPtr->u1, glyphPtr->v1 };
            vert.normal = { 0.f, 0.f, 1.f };
            verts.push_back(vert);

            vert.vertex = { x0, y1, 0.f };
            vert.uv = { glyphPtr->u0, glyphPtr->v1 };
            vert.normal = { 0.f, 0.f, 1.f };
            verts.push_back(vert);

            indices.push_back(idx + 0);
            indices.push_back(idx + 1);
            indices.push_back(idx + 2);
            indices.push_back(idx + 2);
            indices.push_back(idx + 3);
            indices.push_back(idx + 0);

            x += glyphPtr->advance;
        }
        mesh->SetName("GeneratedMesh");
        mesh->SetVertex(std::move(verts));
        mesh->SetIndices(std::move(indices));
        mesh->Build(*world.renderer.GetContext());
        return mesh;
    }
}//namespace
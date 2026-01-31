#include "Component/Render/TextRenderer.h"
#include "World.h"

#include "Core/Util.h"

#include "Render/Model.h"
namespace sh::game
{
    TextRenderer::TextRenderer(GameObject& owner) :
        MeshRenderer(owner)
    {
        auto mat = static_cast<render::Material*>(core::SObject::GetSObjectUsingResolver(core::UUID{"bbc4ef7ec45dce223297a224f8093f23"})); // UITextMat
        assert(mat != nullptr);
        SetMaterial(mat);

        if (IsEditor())
        {
            debugRenderer = gameObject.AddComponent<DebugRenderer>();
            render::Model* cubeModel = static_cast<render::Model*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{ "bbc4ef7ec45dce223297a224f8093f16" })); // Cube Model
            debugRenderer->SetMesh(cubeModel->GetMeshes()[0]);
            debugRenderer->hideInspector = true;
        }
    }
    SH_GAME_API void TextRenderer::OnDestroy()
    {
        if (debugRenderer != nullptr)
        {
            debugRenderer->Destroy();
            debugRenderer = nullptr;
        }
        Super::OnDestroy();
    }
    SH_GAME_API void TextRenderer::Start()
    {
        if (GetMesh() == nullptr)
            Setup();
    }
    SH_GAME_API void TextRenderer::Update()
    {
        Super::Update();

        if (debugRenderer != nullptr)
            UpdateDebugRenderer();

        if (beforeScaleX != gameObject.transform->GetWorldScale().x ||
            beforeScaleY != gameObject.transform->GetWorldScale().y)
        {
            Setup();
            beforeScaleX = gameObject.transform->GetWorldScale().x;
            beforeScaleY = gameObject.transform->GetWorldScale().y;
        }
    }
    SH_GAME_API void TextRenderer::OnPropertyChanged(const core::reflection::Property& prop)
    {
        Super::OnPropertyChanged(prop);

        if (prop.GetName() == core::Util::ConstexprHash("txt") || 
            prop.GetName() == core::Util::ConstexprHash("font") ||
            prop.GetName() == core::Util::ConstexprHash("width"))
        {
            Setup();
        }
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
                SetMesh(mesh);
        }

        if (GetMaterial() == nullptr)
            return;

        auto propBlock = GetMaterialPropertyBlock();
        if (propBlock == nullptr)
            SetMaterialPropertyBlock(std::make_unique<render::MaterialPropertyBlock>());
        propBlock = GetMaterialPropertyBlock();

        propBlock->SetProperty("tex", font->GetAtlases()[0]);
        UpdatePropertyBlockData();
    }
    auto TextRenderer::CreateQuad() -> render::Mesh*
    {
        if (!core::IsValid(font))
            return nullptr;

        const char* start = txt.data();
        const char* end = start + txt.size();
        const float lineHeight = font->GetLineHeightPx();

        float x, y;
        x = 0.f;
        y = -lineHeight;
        uint32_t prevUnicode = 0;

        render::Mesh* mesh = core::SObject::Create<render::Mesh>();

        std::vector<render::Mesh::Vertex> verts;
        std::vector<uint32_t> indices;

        height = lineHeight;
        while (start < end)
        {
            uint32_t unicode = 0;
            start = core::Util::UTF8ToUnicode(start, end, unicode);

            if (unicode == '\r') 
                continue;
            if (unicode == '\n')
            {
                x = 0.f;
                y -= lineHeight;
                height += lineHeight;
                prevUnicode = 0;
                continue;
            }

            auto glyphPtr = font->GetGlyph(unicode);
            if (glyphPtr == nullptr)
                continue;

            const float curWidth = (x + glyphPtr->bearingX + glyphPtr->w) * gameObject.transform->GetWorldScale().x;
            if (curWidth > width)
            {
                x = 0.f;
                y -= lineHeight;
                height += lineHeight;
            }
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
        height *= gameObject.transform->GetWorldScale().y;

        mesh->SetName("GeneratedMesh");
        mesh->SetVertex(std::move(verts));
        mesh->SetIndices(std::move(indices));
        mesh->Build(*world.renderer.GetContext());
        return mesh;
    }
    void TextRenderer::UpdateDebugRenderer()
    {
        if (bDisplayArea)
        {
            if (!debugRenderer->IsActive())
                debugRenderer->SetActive(true);

            const auto& worldPos = gameObject.transform->GetWorldPosition();
            const auto& quat = gameObject.transform->GetWorldQuat();

            const glm::vec3 localCenterOffset{ width * 0.5f, -height * 0.5f, 0.f };
            const glm::vec3 rotatedOffset = quat * localCenterOffset;

            debugRenderer->SetPosition({ worldPos.x + rotatedOffset.x, worldPos.y + rotatedOffset.y, worldPos.z + rotatedOffset.z });
            debugRenderer->SetScale({ width, height, 1.f });
            debugRenderer->SetQuat(quat);
        }
        else if (!bDisplayArea && debugRenderer->IsActive())
            debugRenderer->SetActive(false);
    }
}//namespace
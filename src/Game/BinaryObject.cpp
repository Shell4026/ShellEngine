#include "BinaryObject.h"

namespace sh::game
{
    SH_GAME_API auto BinaryObject::Serialize() const -> core::Json
    {
        core::Json json = Super::Serialize();
        json["BinaryObject"]["data"] = data;
        return json;
    }
    SH_GAME_API void BinaryObject::Deserialize(const core::Json& json)
    {
        Super::Deserialize(json);

        auto it = json.find("BinaryObject");
        if (it != json.end())
        {
            it = it->find("data");
            if (it != json.end())
                data = json.get<std::vector<uint8_t>>();
        }
    }
    SH_GAME_API auto BinaryObject::operator=(const BinaryObject& other) -> BinaryObject&
    {
        data = other.data;

        return *this;
    }
    SH_GAME_API auto BinaryObject::operator=(BinaryObject&& other) noexcept -> BinaryObject&
    {
        data = std::move(other.data);

        return *this;
    }
}//namespace
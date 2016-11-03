#ifndef SP2_GRAPHICS_SCENE_RENDERDATA_H
#define SP2_GRAPHICS_SCENE_RENDERDATA_H

#include <sp2/string.h>
#include <sp2/graphics/shader.h>
#include <SFML/Graphics/RenderTarget.hpp>
#include <vector>
#include <memory>

namespace sp {

class MeshData;
class RenderData
{
public:
    enum class Type
    {
        None,
        Normal,
        Transparent,
        Additive,
    };
    int order;
    Type type;
    Shader* shader;
    std::shared_ptr<MeshData> mesh;
    sf::Color color;
    string texture;
    
    RenderData();
    
    bool operator<(const RenderData& data) const;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_RENDERDATA_H
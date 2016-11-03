#ifndef SP2_GRAPHICS_SCENE_RENDERPASS_H
#define SP2_GRAPHICS_SCENE_RENDERPASS_H

#include <sp2/graphics/graphicslayer.h>
#include <map>

namespace sp {

class RenderPass : public AutoPointerObject
{
public:
    RenderPass(string target_layer);
    
    virtual void render(sf::RenderTarget& target, P<GraphicsLayer> layer) = 0;
    
    string getTargetLayer() const;
    virtual std::vector<string> getSourceLayers() const;
private:
    string target_layer;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_RENDERPASS_H
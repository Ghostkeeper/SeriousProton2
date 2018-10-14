#include <sp2/graphics/renderTexture.h>

namespace sp {

RenderTexture::RenderTexture(sp::string name, Vector2i size, bool double_buffered)
: Texture(Texture::Type::Dynamic, name), double_buffered(double_buffered)
{
    render_texture[0].create(size.x, size.y, true);
    render_texture[0].clear();
    dirty[0] = false;
    if (double_buffered)
    {
        render_texture[1].create(size.x, size.y, true);
        render_texture[1].clear();
        dirty[1] = false;
    }
    
    flipped = false;
}

const sf::Texture* RenderTexture::get()
{
    int index = 0;
    
    if (double_buffered)
        index = flipped ? 0 : 1;
    
    if (dirty[index])
    {
        render_texture[index].display();
        dirty[index] = false;
    }
    return &render_texture[index].getTexture();
}

Vector2i RenderTexture::getSize() const
{
    sf::Vector2u size = render_texture[0].getSize();
    return Vector2i(size.x, size.y);
}

void RenderTexture::activateRenderTarget()
{
    if (double_buffered)
        flipped = !flipped;

    int index = flipped ? 1 : 0;

    dirty[index] = true;
    render_texture[index].setActive();
}

};//namespace sp

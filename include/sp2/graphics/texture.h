#ifndef SP2_GRAPHICS_TEXTURE_H
#define SP2_GRAPHICS_TEXTURE_H

#include <sp2/string.h>
#include <sp2/nonCopyable.h>
#include <sp2/graphics/image.h>
#include <thread>
#include <mutex>


namespace sp {

class Texture : NonCopyable
{
public:
    enum class Type
    {
        Static,
        Dynamic
    };
    virtual void bind() = 0;
    int getRevision() { return revision; }
protected:
    Texture(Type type, string name)
    : type(type), name(name), revision(0) {}
    virtual ~Texture() {}

    Type type;
    string name;
    int revision;
};

class OpenGLTexture : public Texture
{
public:
    OpenGLTexture(Type type, string name);
    
    virtual ~OpenGLTexture();
    
    virtual void bind() override;

protected:
    void setImage(Image&& image);
    
private:
    std::mutex mutex;
    sp::Image image;

    unsigned int handle;
};

};//namespace sp

#endif//SP2_GRAPHICS_TEXTUREMANAGER_H

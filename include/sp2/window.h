#ifndef SP2_WINDOW_H
#define SP2_WINDOW_H

#include <sp2/math/vector.h>
#include <sp2/pointer.h>
#include <sp2/pointerList.h>
#include <sp2/string.h>
#include <sp2/io/pointer.h>
#include <sp2/graphics/color.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <map>
#include <memory>

struct SDL_Window;
union SDL_Event;

namespace sp {
namespace io { class Clipboard; }

class GraphicsLayer;
class Texture;
class MeshData;
class Window : public AutoPointerObject
{
public:
    Window();
    Window(float aspect_ratio);
    Window(Vector2f size_factor);
    Window(Vector2f size_factor, float aspect_ratio);
    
    void setFullScreen(bool fullscreen);
    void setClearColor(Color color);
    void hideCursor();
    void setDefaultCursor();
    void setCursor(Texture* texture, std::shared_ptr<MeshData> mesh);
    void setPosition(Vector2f position);
    
    void addLayer(P<GraphicsLayer> layer);
    
    void close();
private:
    SDL_Window* render_window;
    void* render_context;
    RenderQueue queue;

    virtual ~Window();
    
    void createRenderWindow();
    void render();
    void handleEvent(const SDL_Event& event);
    void pointerDown(io::Pointer::Button button, Vector2d position, int id);
    void pointerDrag(Vector2d position, int id);
    void pointerUp(Vector2d position, int id);
    Vector2d screenToGLPosition(int x, int y);
    
    PList<GraphicsLayer> graphics_layers;
    
    string title;
    int antialiasing;
    bool fullscreen;
    Color clear_color;
    Texture* cursor_texture;
    std::shared_ptr<MeshData> cursor_mesh;

    //Screen size/position information.
    float window_aspect_ratio; //If != 0 then this is the forced aspect ratio in window mode (ignored in full screen)
    Vector2f max_window_size_ratio; //If != 0 then this is the maximum size the window may take of screen, in faction (0.0 to 1.0)
    
    int mouse_button_down_mask;
    std::map<int, P<GraphicsLayer>> pointer_focus_layer;

    static PList<Window> windows;
    static bool anyWindowOpen();
    
    //Engine needs to be able to access the render window, as this is needed for event polling.
    friend class Engine;
    friend class GraphicsLayer;
    friend class io::Clipboard;
};

};//namespace sp

#endif//SP2_WINDOW_H

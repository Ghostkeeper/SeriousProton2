#ifndef SP2_GRAPHICS_SPRITE_ANIMATION_H
#define SP2_GRAPHICS_SPRITE_ANIMATION_H

#include <sp2/graphics/scene/renderdata.h>
#include <sp2/graphics/animation.h>
#include <sp2/math/vector.h>
#include <sp2/string.h>

namespace sp {

class SpriteAnimation : public Animation
{
public:
    virtual void play(string key, float speed=1.0) override;
    virtual void setFlags(int flags) override;
    virtual int getFlags() override;
protected:
    virtual void prepare(RenderData& render_data) override;
    virtual void update(float delta, RenderData& render_data) override;

private:
    class Data
    {
    public:
        class Animation
        {
        public:
            class Frame
            {
            public:
                std::shared_ptr<MeshData> normal_mesh;
                std::shared_ptr<MeshData> mirrored_mesh;
                float delay;
            };
            
            Texture* texture;
            std::vector<Frame> frames;
            bool loop;
        };
        
        std::map<string, Animation> animations;
#ifdef DEBUG
        string resource_name;
        std::chrono::system_clock::time_point resource_update_time;
        int revision;
#endif

        void load(string resource_name);
    };

    SpriteAnimation(const Data& data);

    const Data& data;
    const Data::Animation* animation;
    float time_delta;
    float speed;
    bool playing;
    unsigned int keyframe;
    bool flip;
#ifdef DEBUG
    int revision;
#endif
public:
    static std::unique_ptr<Animation> load(string resource_name);
    static constexpr int FlipFlag = 0x0001;
private:
    static std::map<string, Data*> cache;
};

};//namespace sp

#endif//SP2_GRAPHICS_SPRITE_ANIMATION_H

#ifndef POINT_SPRITE_RENDERER_H
#define POINT_SPRITE_RENDERER_H
#include "nuc_renderer.h"

class PointSpriteRenderer:public NucRenderer
{
private:
    void flush_particles();

public:
    PointSpriteRenderer();
    ~PointSpriteRenderer();

    virtual void init();
    virtual void bind_attributes();

    virtual void pre_render_ops();
    virtual void render(const std::list<NucParticle *> &particles,
                goatgfx::Texture *tex, float size);
    virtual void post_render_ops();
};

#endif // POINT_SPRITE_RENDERER_H

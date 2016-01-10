#ifndef POINT_SPRITE_ORTHO_RENDERER_H
#define POINT_SPRITE_ORTHO_RENDERER_H
#include "point_sprite_renderer.h"

class PointSpriteOrthoRenderer:public PointSpriteRenderer
{
private:
    Matrix4x4 prev_view_mat;
    Matrix4x4 prev_proj_mat;

public:
    void pre_render_ops();
    void post_render_ops();
};

#endif // POINT_SPRITE_ORTHO_RENDERER_H

#include "point_sprite_ortho_renderer.h"
#include "game.h"

void PointSpriteOrthoRenderer::pre_render_ops()
{
    prev_view_mat = get_view_matrix();
    prev_proj_mat = get_projection_matrix();

    Matrix4x4 identity_view;
    set_view_matrix(identity_view);

    Matrix4x4 ortho_proj;
    ortho_proj.set_orthographic(0, game::options::win_width, game::options::win_height, 0, -100.0, 100.0);
    set_projection_matrix(ortho_proj);
}

void PointSpriteOrthoRenderer::post_render_ops()
{
    set_view_matrix(prev_view_mat);
    set_projection_matrix(prev_proj_mat);
}

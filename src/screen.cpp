#include "screen.h"
#include "unistate.h"
#include "game.h"

BaseScreen::BaseScreen()
{
    timer.stop();
    prev_sec = dt = 0;

    prev_click_time = 0;
    dbl_click_threshold = 250; //get this from a config file.
    drag_acceleration = 0;

    frames = 0;
    prev_sample_time = 0;
}

BaseScreen::~BaseScreen()
{

}

void BaseScreen::on_mouse_down(int btn, int x, int y)
{
    prev_click_time = get_msec();
    reset_drag();
    cursor_pos_pressed = Vector2(x, y);
}

void BaseScreen::on_mouse_up(int btn, int x, int y)
{
    cursor_pos_released = Vector2(x, y);
}

void BaseScreen::on_double_click(int btn, int x, int y)
{

}

void BaseScreen::on_move(int x, int y)
{

}

void BaseScreen::on_drag(int x, int y)
{
    calc_drag(x, y);
}

void BaseScreen::on_key_press(unsigned char key, int x, int y)
{

}

void BaseScreen::on_key_release(unsigned char key, int x, int y)
{

}

bool BaseScreen::init()
{
    frames = 0;
    prev_sample_time = 0;

    return true;
}

void BaseScreen::start()
{
    timer.start();
}

void BaseScreen::reset()
{
    timer.reset();
	prev_sec = 0;
	dt = 0;
}

void BaseScreen::stop()
{
    timer.stop();
}

long BaseScreen::get_msec()
{
    return timer.get_msec();
}

float BaseScreen::get_sec()
{
    return timer.get_sec();
}

void BaseScreen::calc_dt()
{
    float sec = get_sec();
    dt = sec - prev_sec;
    prev_sec = sec;
}

bool BaseScreen::has_started()
{
    if(timer.is_running())
        return true;
    return false;
}

void BaseScreen::set_drag_acceleration(float drag_accel)
{
    drag_acceleration = drag_accel;
}

float BaseScreen::get_drag_acceleration() const
{
    return drag_acceleration;
}

long BaseScreen::get_prev_click_time() const
{
    return prev_click_time;
}

void BaseScreen::set_dbl_click_threshold(long thresh)
{
    dbl_click_threshold = thresh;
}

long BaseScreen::get_dbl_click_threshold() const
{
    return dbl_click_threshold;
}

void BaseScreen::calc_drag(int x, int y)
{
    drag = Vector2(x, y) - cursor_pos_pressed;
}

void BaseScreen::reset_drag()
{
    drag.x = 0;
    drag.y = 0;
}

void BaseScreen::calc_ms_per_frame(long sample_interval_msec)
{
    long time = get_msec();

    frames++;

    if(time - prev_sample_time > 2000) {
        std::cout<<"ms/frame:"<< (double)sample_interval_msec / (double)frames << std::endl;
        prev_sample_time = time;
        frames = 0;
    }
}

/*Private Functions-------------------------*/
Ray BaseScreen::calc_picking_ray(int x, int y)
{
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    Matrix4x4 view = get_view_matrix();
    Matrix4x4 proj = get_projection_matrix();

    Vector3 p_near = game::utils::unproject(x, y, 0, proj, view, vp);
    Vector3 p_far = game::utils::unproject(x, y, 1, proj, view, vp);

    return Ray(p_near, p_far - p_near);
}

/*------------------------------------------*/


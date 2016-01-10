#ifndef SCREEN_H
#define SCREEN_H
#include "vmath.h"
#include "timer.h"
#include "event_handler.h"

class BaseScreen
{
private:
    Timer timer;

protected:
    Matrix4x4 projection_mat;
    float prev_sec;
    float dt;

    /*msec/frame variables*/
    long prev_sample_time;
    long frames;
    /*-------------------------*/

    long prev_click_time;
    Vector2 cursor_pos_pressed;
    Vector2 cursor_pos_released;
    float drag_acceleration;
    long dbl_click_threshold;
    Vector2 drag;

    Ray calc_picking_ray(int x, int y);

public:
    BaseScreen();
    virtual ~BaseScreen();

    virtual bool init();
    virtual void render() = 0;
    virtual void on_mouse_down(int btn, int x, int y);
    virtual void on_mouse_up(int btn, int x, int y);
    virtual void on_double_click(int btn, int x, int y);
    virtual void on_move(int x, int y);
    virtual void on_drag(int x, int y);
    virtual void on_key_press(unsigned char key,int x,int y);
    virtual void on_key_release(unsigned char key,int x,int y);

    virtual void start();
    virtual void stop();
    virtual void reset();
    virtual void reshape(int x,int y) = 0;
    virtual void update() = 0;

    long get_msec();
    float get_sec();
    virtual void calc_dt();
    bool has_started();

    void set_drag_acceleration(float drag_accel);
    float get_drag_acceleration() const;

    long get_prev_click_time() const;

    void set_dbl_click_threshold(long thresh);
    long get_dbl_click_threshold() const;


    void calc_drag(int x, int y);
    void reset_drag();

    void calc_ms_per_frame(long sample_interval_msec);
};

#endif // SCREEN_H

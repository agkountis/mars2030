#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H
#include <stack>
#include <iostream>
#include "screen.h"

class ScreenManager
{
private:
    std::stack<BaseScreen*> screens;

public:
    ScreenManager();
    ~ScreenManager();

    void push_screen(BaseScreen *scrn);
    void pop_screen();
    void render();
    void on_mouse_down(int btn, int x, int y);
    void on_mouse_up(int btn, int x, int y);
    void on_double_click(int btn, int x, int y);
    void on_drag(int x, int y);
    void on_move(int x, int y);
    void on_key_press(unsigned char key, int x, int y);
    void on_key_release(unsigned char key, int x, int y);
    void reshape(int x,int y);
    void update();

    BaseScreen *get_active_screen() const;
    long get_active_screen_msec() const;
    double get_active_screen_sec() const;

    bool scrn_has_started();
    bool init_active_scrn();
    void calc_screen_dt();
    void start_active_scrn();
    void stop_active_scrn();
};

#endif // SCREEN_MANAGER_H

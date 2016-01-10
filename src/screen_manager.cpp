#include "screen_manager.h"

ScreenManager::ScreenManager()
{

}

ScreenManager::~ScreenManager()
{
}

void ScreenManager::push_screen(BaseScreen *scrn)
{
    if(scrn)
        screens.push(scrn);
}

void ScreenManager::pop_screen()
{
    if(screens.size() > 1){
        screens.pop();
    }
    else{
        std::cout<<"Can't pop the last screen!"<<std::endl;
    }
}

void ScreenManager::render()
{
    if(screens.top()) {
        screens.top()->render();
        //screens.top()->calc_ms_per_frame(2000);
    }
}

void ScreenManager::on_mouse_down(int btn, int x, int y)
{
    if(screens.top())
        screens.top()->on_mouse_down(btn, x, y);
}

void ScreenManager::on_mouse_up(int btn, int x, int y)
{
    if(screens.top())
        screens.top()->on_mouse_up(btn, x, y);
}

void ScreenManager::on_double_click(int btn, int x, int y)
{
    if(screens.top())
        screens.top()->on_double_click(btn, x, y);
}

void ScreenManager::on_drag(int x, int y)
{
    if(screens.top())
        screens.top()->on_drag(x, y);
}

void ScreenManager::on_move(int x, int y)
{
    if(screens.top())
        screens.top()->on_move(x, y);
}

void ScreenManager::on_key_press(unsigned char key, int x, int y)
{
    if(screens.top())
        screens.top()->on_key_press(key, x, y);
}

void ScreenManager::on_key_release(unsigned char key, int x,int y)
{
    if(screens.top())
        screens.top()->on_key_release(key, x, y);
}

void ScreenManager::reshape(int x, int y)
{
    if(screens.top())
        screens.top()->reshape(x, y);
}

void ScreenManager::update()
{
    if(screens.top())
        screens.top()->update();
}

BaseScreen *ScreenManager::get_active_screen() const
{
    if(screens.size() > 0)
        return screens.top();
    return NULL;
}

long ScreenManager::get_active_screen_msec() const
{
    if(screens.top())
        return screens.top()->get_msec();

    return 0;
}

double ScreenManager::get_active_screen_sec() const
{
    if(screens.top())
        return screens.top()->get_sec();

    return 0;
}

bool ScreenManager::scrn_has_started()
{
    if(!screens.top()->has_started())
        return false;
    return true;
}

bool ScreenManager::init_active_scrn()
{
    if(!screens.top()->init()){
        std::cout<<"Could not initialize the active screen!"<<std::endl;
        return false;
    }
    return true;
}

void ScreenManager::calc_screen_dt()
{
    if(screens.top())
        screens.top()->calc_dt();
}

void ScreenManager::start_active_scrn()
{
    if(screens.top())
        screens.top()->start();
}

void ScreenManager::stop_active_scrn()
{
    if(screens.top())
        screens.top()->stop();
}

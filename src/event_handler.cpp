#include "event_handler.h"
#include "game.h"

EventHandler::EventHandler()
{
    mouse_button_state = -1;
}

/*Private Functions*/
void EventHandler::global_on_mouse_down(int btn, int x, int y)
{

}

void EventHandler::global_on_mouse_up(int btn, int x, int y)
{

}

void EventHandler::global_on_key_press(unsigned char key, int x, int y)
{
	switch (key){
	case 27:
		game::engine::exit_game = true;
		break;
	case 'o':
		game::options::ovr_enabled = !game::options::ovr_enabled;
		break;
	case 'f':
		game::options::fullscreen = !game::options::fullscreen;
		break;
	}
}

void EventHandler::global_on_key_release(unsigned char key, int x, int y)
{

}
/*-----------------------------------------------------*/


void EventHandler::on_mouse_down(int btn, int x, int y)
{
   if(detect_bouble_click(btn))
       on_double_click(btn, x, y);

    game::engine::scrn_manager->on_mouse_down(btn, x, y);
}

void EventHandler::on_mouse_up(int btn, int x, int y)
{
    game::engine::scrn_manager->on_mouse_up(btn, x, y);
}

void EventHandler::on_double_click(int btn, int x, int y)
{
    game::engine::scrn_manager->on_double_click(btn, x, y);
}

void EventHandler::on_drag(int x, int y)
{
    game::engine::scrn_manager->on_drag(x, y);
}

void EventHandler::on_move(int x, int y)
{
    game::engine::scrn_manager->on_move(x, y);
}

bool EventHandler::detect_bouble_click(int btn) const
{
    if(btn == MOUSE_LEFT){
         long time = game::engine::scrn_manager->get_active_screen_msec();
         BaseScreen *scrn = game::engine::scrn_manager->get_active_screen();
         long prev_click = scrn->get_prev_click_time();

         long dt = time - prev_click;

         if(dt < scrn->get_dbl_click_threshold() && dt > 0){
             return true;
         }
     }

    return false;
}

void EventHandler::set_mouse_btn_state(int state)
{
    mouse_button_state = state;
}

int EventHandler::get_mouse_btn_state() const
{
    return mouse_button_state;
}

void EventHandler::on_key_press(unsigned char key, int x, int y)
{
	global_on_key_press(key, x, y);
    game::engine::scrn_manager->on_key_press(key, x, y);
}

void EventHandler::on_key_release(unsigned char key, int x, int y)
{
    game::engine::scrn_manager->on_key_release(key, x, y);
}

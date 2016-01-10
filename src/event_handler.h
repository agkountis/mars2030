#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H
#include "vmath.h"

enum MouseButton{
    MOUSE_LEFT = 0, MOUSE_MIDDLE, MOUSE_RIGHT,
    NUM_MOUSE_BTN
};

enum ButtonState{
  BUTTON_PRESSED = 0, BUTTON_RELEASED,
    NUM_MOUSE_BTN_STATES
};

class EventHandler
{
private:
    int mouse_button_state;

	/*Functions to handle events on a global scale. e.g to change some global variable.*/
	void global_on_mouse_down(int btn, int x, int y);
	void global_on_mouse_up(int btn, int x, int y);
	void global_on_key_press(unsigned char key, int x, int y);
	void global_on_key_release(unsigned char key, int x, int y);

public:
    EventHandler();

    /*Mouse Events-------------------------------*/
    void on_mouse_down(int btn, int x, int y);
    void on_mouse_up(int btn, int x, int y);
    void on_double_click(int btn, int x, int y);
    void on_drag(int x, int y);
    void on_move(int x, int y);
    bool detect_bouble_click(int btn) const;

    void set_mouse_btn_state(int state);
    int get_mouse_btn_state() const;
    /*-------------------------------------------*/

    /*Keyboard Events----------------------------*/
    void on_key_press(unsigned char key, int x, int y);
    void on_key_release(unsigned char key, int x, int y);
    /*-------------------------------------------*/
};

#endif // EVENT_HANDLER_H

#ifndef GAME_SCREEN_H_
#define GAME_SCREEN_H_
#include "game.h"
#include "render_target.h"

class GameScreen :public BaseScreen
{
private:
    Matrix4x4 *camera;
	Matrix4x4 shad_mat;
	Matrix4x4 shad_mat2;

    float theta, phi;

    void render_default();
    void render_ovr();

	void shadow_render_pass(bool ovr, long time = 0);
	void color_render_pass(long time = 0);
    void draw_shad_tex();

public:
	GameScreen();
	~GameScreen();

	bool init();
	void render();
    void on_mouse_down(int btn, int x, int y);
    void on_mouse_up(int btn, int x, int y);
	void on_double_click(int btn, int x, int y);
	void on_move(int x, int y);
	void on_drag(int x, int y);
	void on_key_press(unsigned char key, int x, int y);
    void on_key_release(unsigned char key, int x, int y);
	void reshape(int x, int y);
	void update();
};

#endif

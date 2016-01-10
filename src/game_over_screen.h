#ifndef GAME_OVER_SCREEN_H_
#define GAME_OVER_SCREEN_H_
#include <list>
#include "screen.h"
#include "scene.h"
#include "nuc_emitter.h"
#include "bitmap_text.h"

class GameOverScreen :public BaseScreen
{
private:
	std::list<Scene*> scenes;
	std::map<std::string, Scene*> game_over_scrn_scenes_by_name;
	Matrix4x4 view;
	BitmapText game_over_txt;
	Light *lt;
	Object sky;
	NucEmitter *expl_emitter;
	bool expl_done;
	bool draw_ship;

	void render_default();
	void render_ovr();

public:

	GameOverScreen();
	~GameOverScreen();

	void add_scene(Scene *scene);

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
	void map_scene_by_name(std::string name, Scene *sc);
};

#endif

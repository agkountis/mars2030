#ifndef STARTING_SCREEN_H_
#define STARTING_SCREEN_H_
#include <list>
#include "screen.h"
#include "scene.h"
#include "bitmap_text.h"
#include "render_target.h"
#include "audio_manager.h"

class StartingScreen :public BaseScreen
{
private:
	std::list<Scene*> scenes;
	std::map <std::string, Scene*> starting_scrn_scenes_by_name;
	Matrix4x4 view;
	RenderTarget billboard_scrn_rtarg;
	goatgfx::Texture *billboard_tex;
	BitmapText title;
	BitmapText start_message;
	Light *lt;
	Object sky;
	StreamPlaylist plist;

	void render_billboard_scrn_tex();
	void render_default();
	void render_ovr();

public:
	StartingScreen();
	~StartingScreen();

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

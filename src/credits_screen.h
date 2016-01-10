#ifndef CREDITS_SCREEN
#define CREDITS_SCREEN
#include <list>
#include <map>
#include <stack>
#include "scene.h"
#include "screen.h"
#include "bitmap_text.h"
#include "nuc_emitter.h"
#include "audio_manager.h"

class CreditsScreen:public BaseScreen {

private:
    std::list<Scene*> scenes;
	std::map<std::string, Scene*> credits_scrn_scenes_by_name;

	BitmapText *credits_txt;
	std::stack<std::string> strings;
	long start_time;

    Light *lt;
    Object sky;
    Matrix4x4 view;

	StreamPlaylist playlist;

    long duration;

    int stream_idx;
    bool skip;

    NucEmitter *em;

    void render_default();
    void render_ovr();

public:
    CreditsScreen();
    ~CreditsScreen();

    bool init();
    void render();
    void on_mouse_down(int btn, int x, int y);
	void on_move(int x, int y);
	void on_key_press(unsigned char key, int x, int y);

    void reshape(int x,int y);
    void update();

	void add_scene(Scene *sc);
	void map_scene_by_name(std::string name, Scene *sc);
};

#endif // CREDITS_SCREEN


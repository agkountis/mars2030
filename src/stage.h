#ifndef STAGE_H_
#define STAGE_H_
#include <iostream>
#include <list>
#include "scene.h"
#include "wave.h"
#include "projectile.h"
#include "source.h"
#include "wave_group_manager.h"
#include "audio_manager.h"
#include "holomap.h"
#include "render_target.h"
#include "bitmap_text.h"
#include "weapon_system.h"

enum StageRenderTypes {
    ALL = 0, SKY, SCENES, ENEMIES, GUNS, COCKPIT, PROJECTILES, HOLOMAP, COCKPIT_SCRN_TEXTURES, 
	RENDER_HUD, RENDER_HUD_OVR, RENDER_CROSSHAIR
};

#define STAGE_RENDER_ALL 0xffffffff
#define STAGE_RENDER_SKY (1 << SKY)
#define STAGE_RENDER_SCENES (1 << SCENES)
#define STAGE_RENDER_ENEMIES (1 << ENEMIES)
#define STAGE_RENDER_GUNS (1 << GUNS)
#define STAGE_RENDER_COCKPIT (1 << COCKPIT)
#define STAGE_RENDER_PROJECTILES (1 << PROJECTILES)
#define STAGE_RENDER_HOLOMAP (1 << HOLOMAP)
#define STAGE_RENDER_COCKPIT_SCRN_TEXTURES (1 << COCKPIT_SCRN_TEXTURES)
#define STAGE_RENDER_HUD (1 << RENDER_HUD)
#define STAGE_RENDER_HUD_OVR (1 << RENDER_HUD_OVR)
#define STAGE_RENDER_CROSSHAIR (1 << RENDER_CROSSHAIR)

class Stage
{
private:
    std::list<Scene*> stage_scenes;
	std::map<std::string, Scene*> scene_ptr_by_name;
	WaveGroupManager wave_group_manager;

    Matrix4x4 camera;
    Light *lt;
    Object sky;
	AudioSource source;

	/*Weapon System*/
	WeaponSystem weapon_system;
	/*----------------------------------*/

	/*Scenes than need special handling.*/
	Scene *cockpit_sc;
	/*-----------------------------------*/

    /*Used to manipulate the model's
     * rotation based on the view matrix*/
    XFormNode *cockpit_ctrl;
	/*-----------------------------------*/

	/*Audio*/
	AudioSample *tutorial;
	AudioSource *tutorial_src;
	bool skip_tutorial;
	bool tutorial_started;
	bool tutorial_finished;

	StreamPlaylist playlist;
	/*----------------*/

	/*The Holomap!*/
	Holomap holomap;
	/*--------------*/

	/*Cockpit's Screens render targets and background texture.*/
	RenderTarget left_scrn_rtarg;
	RenderTarget right_scrn_rtarg;
	RenderTarget far_right_scrn_rtarg;
	RenderTarget far_left_scrn_rtarg;
	unsigned int background;
	/*---------------------------------*/

	/*Stage health*/
	int health;
	/*------------------------*/

	/*Screen texts*/
	BitmapText text;
	BitmapText hud_text;
	XFormNode hud_ctrl_node;
	/*---------------------------------*/

	/*Crosshair*/
	Object crosshair;

    /*Stage Beaten*/
    bool stage_over;


    bool load_scene(const char *fname);
	bool load_waves(const NCF &ncf);
	bool load_cockpit(const NCF &ncf); //TODO
	bool load_guns(const NCF &ncf); //TODO
    void setup_psys_emitters();

    void render_sky(long time = 0);
    void render_scenes(long time = 0);
    void render_enemies(long time = 0);
	void render_guns (long time = 0);
	void render_cockpit(long time = 0);
	void render_projectiles(long time = 0);
	void render_holomap(long time = 0);
	void render_cockpit_screens(long time = 0);
	void render_hud(long time = 0);
	void render_hud_ovr(long time = 0);
	void render_crosshair(long time = 0);

public:
    Stage();
    ~Stage();

	long kill_count;

	void set_health(int hp);
	void set_shooting_state(bool state);
	int get_health() const;

    Matrix4x4 *get_camera_matrix();
	XFormNode *get_gun_ctrl_node_ptr();
	XFormNode *get_cockpit_ctrl_node_ptr();
	Scene *get_cockpit_scene_ptr();
	Holomap *get_holomap_ptr();
	XFormNode *get_hud_ctrl_node_ptr();
	WeaponSystem *get_weapon_system_ptr();
	Object *get_crosshair();
	Scene* get_gun(unsigned int pos_enum);
	WaveGroupManager *get_wave_group_manager_ptr();
	Light *get_light();

	void set_skip_tutorial_state(bool state);
	bool has_tutorial_started() const;


    bool load(const char *fname);
    bool init();
    void setup_lights();
	void render(unsigned int render_mask, long time = 0);
    void update(long time, float dt);
	void setup_psys_emitters(Scene *sc);
};

#endif

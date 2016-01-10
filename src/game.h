#ifndef GAME_H_
#define GAME_H_

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "ovr_manager.h"
#include "sdrman.h"
#include "vmath.h"
#include "unistate.h"
#include "timer.h"
#include "cfg.h"
#include "screen_manager.h"
#include "nuc_emitter.h"
#include "nuc_physics.h"
#include "point_sprite_renderer.h"
#include "point_sprite_ortho_renderer.h"
#include "nuc_manager.h"
#include "camera.h"
#include "stage.h"
#include "enemy_template.h"
#include "imago2.h"
#include "render_target.h"
#include "audio.h"
#include "audio_manager.h"
#include "drawtext.h"

enum RenderPassType {
	RNDR_PASS_COLOR, RNDR_PASS_SHADOW
};

namespace game
{
    namespace assets
    {
        /*Enemy Templates and maps*/
        extern std::vector<EnemyTemplate*> enemy_templates;

        extern std::map<std::string, unsigned int> enemy_idx_by_name;
        /*----------------------------------*/

        /*Dummy textures for debugging mainly*/
        extern goatgfx::Texture *diff_dummy;
        extern goatgfx::Texture *spec_dummy;
        extern goatgfx::Texture *norm_dummy;
        /*-----------------------------------*/

		/*Audio Assets------------------------------------------------*/
		//Sound effects and short dialogs.
		extern std::vector<AudioSample*> audio_samples;
		extern std::map<std::string, unsigned int> audio_sample_idx_by_name;
		/*-----------------------------------------------------------*/

		extern dtx_font *font;
    }

    namespace options
    {
        /*Window parameters*/
        extern int win_width;
        extern int win_height;
        /*----------------------------------*/

        /*Anti-Aliasing factors and switches*/
        extern float ss_factor;
        /*-----------------------------------*/

        /*Fullscreen switch*/
        extern bool fullscreen;
        /*------------------------------------*/

        /*OculusVR enable switch*/
        extern bool ovr_enabled;
        /*----------------------------------*/

        /*View (Camera) parameters*/
        extern float vertical_fov;
        extern float max_fov; //Vertical or horizontal.
        /*----------------------------------*/
    }

    namespace engine
    {
        /*All the screens*/
        extern BaseScreen *logo_scrn;
        extern BaseScreen *starting_scrn;
        extern BaseScreen *game_scrn;
		extern BaseScreen *game_over_scrn;
        extern BaseScreen *credits_scrn;
        /*----------------------------------*/

		/*Quad used to display the rendered texture to the screen*/
		extern Mesh *rendering_quad;
		/*-----------------------------------------------------------*/

        /*Shaders and the current active shader pointer*/
        extern ShaderProg *main_sdr;
        extern ShaderProg *cubemap_sdr;
        extern ShaderProg *shadow_pass_sdr;
        extern ShaderProg *point_sprite_sdr;
        extern ShaderProg *fb_color_sdr;
        extern ShaderProg *fb_color_sdr_fxaa;
		extern ShaderProg *fresnel_sdr;
        extern ShaderProg *current_sdr;
		extern ShaderProg *depth_pass_sdr;
		extern ShaderProg *depth_dbg_sdr;
		extern ShaderProg *main_no_shad_sdr;
		extern ShaderProg *holomap_sdr;
		extern ShaderProg *holomap_4dnoise_sdr;
		extern ShaderProg *font_sdr;
		extern ShaderProg *no_lighting_sdr;
		extern ShaderProg *crosshair_sdr;
		extern ShaderProg *flashing_text_sdr;
		extern ShaderProg *atmosphere_sdr;
		extern ShaderProg *dimmed_sky_sdr;
		extern ShaderProg *drone_shield_sdr;
		extern ShaderProg *point_sprite_heat_sdr;
        /*---------------------------------------------*/

        /*Particles System maps and configurations*/
        extern std::vector<NucEmitterConfig> emitter_configs;
        extern std::vector<NucPhysics*> physics_sims;

        extern std::map<std::string, unsigned int> emitter_config_idx_by_name;
        extern std::map<std::string, unsigned int> physics_sim_idx_by_name;
        /*---------------------------------------------*/

        /*Renderers*/
        extern PointSpriteRenderer *ps_r;
        extern PointSpriteOrthoRenderer *ps_ortho_r;
		extern PointSpriteRenderer *ps_heat_renderer;
        /*---------------------------------------------*/

        /*Texture set (manages textures)*/
        extern goatgfx::TextureSet *texset;
        /*----------------------------------*/

        /*Managers and Handlers*/
        extern NucManager *nuc_manager;
        extern ScreenManager *scrn_manager;
		extern AudioManager *audio_manager;

        extern EventHandler *event_handler;
        /*----------------------------------*/

        /*The currently active stage.*/
        extern Stage *active_stage;
        /*----------------------------------*/

        /*Program Exit Switch*/
        extern bool exit_game;
        /*---------------------*/

        /*control flags*/
        extern bool warp_mouse;
        /*----------------------------------*/

        /*Render targets*/
        extern RenderTarget rtarg;
		extern RenderTarget rtarg2;
        /*------------------------------------*/

		/*Ovr Manager*/
		extern OvrManager ovr_manager;
		/*---------------------------------------*/

		/*Ovr initialization flag*/
        extern bool ovr_initialized;
		/*--------------------------------------*/

		/*Render pass type enumeration*/
		extern RenderPassType rndr_pass_type;
		/*----------------------------------------*/

		/*Generates geometry for the rendering quad.*/
		void generate_rendering_quad();
		/*------------------------------------------*/

		/*Updates the texture coordinates of the rendering quad in case of window resizing*/
		void update_rendering_quad_tc();
		/*--------------------------------------------------------------------------------*/
    }

    namespace utils
    {
        struct SphericalCoord
        {
            float r;
            float phi;
            float theta;

            SphericalCoord();
            SphericalCoord(float r , float phi , float theta );
        };

        /*Uprojects a 2D vector. (Moves it to world space) */
        Vector3 unproject(int win_x, int win_y, int win_z, const Matrix4x4 &proj, const Matrix4x4 &view, int *viewport);

        /*Return the value of the input raised to the next nearest power of 2*/
        unsigned int next_pow2(unsigned int num);

        SphericalCoord cartesian_to_spherical(const Vector3 &v);

        /*Reference plane is the Y plane.
		*Vertical angle: 0
		*Horizontal angle: 0
		*Radius: 1
		*Cartesian coordinate result (x, y, z): (0, 1, 0)*/
        Vector3 spherical_to_cartesian_y_ref(float vertical_angle, float horizontal_angle, float radius = 1.0);

		/*Reference plane is the Z plane.
		*Vertical angle: 0
		*Horizontal angle: 0
		*Radius: 1
		*Cartesian coordinate result (x, y, z): (0, 0, -1)*/
		Vector3 spherical_to_cartesian_z_ref(float vertical_angle, float horizontal_angle, float radius = 1.0);

        void gen_uv_sphere(Mesh *m, float radius, int usub , int vsub , float urange , float vrange);
        void gen_cube(Mesh *m, float x_size, float y_size, float z_size, const Vector3 &pivot_loc);

		EnemyTemplate *get_enemy_template_by_name(const std::string &name);
		AudioSample *get_audio_sample_by_name(const std::string &name);
		void create_scene_psys_emitters(Object *obj);
        void display_to_rendering_quad();
    } //utils namespace

	/*game initialization*/
	bool init_shaders();
	bool init_managers();
	bool init_handlers();
	bool init_screens();
	bool init_dummy_textures();
	bool pre_load_init();
	bool post_load_init();
    bool load(const char *fname);
	/*----------------------------------*/

    void cleanup();
    void cleanup_assets();
    void cleanup_screens();
    void cleanup_stages();
    void cleanup_managers();
    void cleanup_handlers();
    void cleanup_physics_sims();
    void cleanup_shaders();

}

#endif //GAME_H_

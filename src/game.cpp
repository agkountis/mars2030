#include "game.h"
#include "game_screen.h"
#include "starting_screen.h"
#include "game_over_screen.h"
#include "credits_screen.h"

/*loads the enemy templates*/
static bool load_enemy_tmpl(const char *fname);

/*loads the config values of the Psys emitters*/
static bool load_emitter_configs(const char *fname);

/*loads the config values of the psys physics simulators*/
static bool load_physics_sim_configs(const char *fname);

/*adds the standart state uniforms to the global shader uniform state*/
static void add_standart_state_uniforms();

/*binds the mesh attributes to the specified shader*/
static void bind_attributes(ShaderProg *sdr);

/*adds the datapaths for file loading*/
static void add_datapaths();

namespace game
{
    namespace assets
    {
        /*Enemy Templates and maps*/
        std::vector<EnemyTemplate*> enemy_templates;
        std::map<std::string, unsigned int> enemy_idx_by_name;
        /*----------------------------------*/

        /*Dummy textures for debugging mainly*/
         goatgfx::Texture *diff_dummy = NULL;
         goatgfx::Texture *spec_dummy = NULL;
         goatgfx::Texture *norm_dummy = NULL;
        /*-----------------------------------*/

		 /*Audio Assets------------------------------------------------*/
		 //Sound effects and short dialogs.
		 std::vector<AudioSample*> audio_samples;
		 std::map<std::string, unsigned int> audio_sample_idx_by_name;
		 /*-----------------------------------------------------------*/

		 dtx_font *font;
    }

    namespace options
    {
        /*Window parameters*/
        int win_width;
        int win_height;
        /*----------------------------------*/

        /*Anti-Aliasing factors and switches*/
        float ss_factor;
        /*-----------------------------------*/

        /*Fullscreen switch*/
        bool fullscreen;
        /*------------------------------------*/

        /*OculusVR enable switch*/
        bool ovr_enabled;
        /*----------------------------------*/

        /*View (Camera) parameters*/
        float vertical_fov;
        float max_fov; //Vertical or horizontal.
        /*----------------------------------*/
    }

    namespace engine
    {
        /*All the screens*/
        BaseScreen *logo_scrn = NULL;
        BaseScreen *starting_scrn = NULL;
        BaseScreen *game_scrn = NULL;
		BaseScreen *game_over_scrn = NULL;
        BaseScreen *credits_scrn = NULL;
        /*----------------------------------*/

		/*Quad used to display the rendered texture to the screen*/
		Mesh *rendering_quad = NULL;
		/*-----------------------------------------------------------*/

        /*Shaders and the current active shader pointer*/
        ShaderProg *main_sdr = NULL;
        ShaderProg *cubemap_sdr = NULL;
        ShaderProg *shadow_pass_sdr = NULL;
        ShaderProg *point_sprite_sdr = NULL;
        ShaderProg *fb_color_sdr = NULL;
        ShaderProg *fb_color_sdr_fxaa = NULL;
		ShaderProg *fresnel_sdr = NULL;
        ShaderProg *current_sdr = NULL;
		ShaderProg *depth_pass_sdr = NULL;
		ShaderProg *depth_dbg_sdr = NULL;
		ShaderProg *main_no_shad_sdr = NULL;
		ShaderProg *holomap_sdr = NULL;
		ShaderProg *holomap_4dnoise_sdr = NULL;
		ShaderProg *font_sdr = NULL;
		ShaderProg *no_lighting_sdr = NULL;
		ShaderProg *crosshair_sdr = NULL;
		ShaderProg *flashing_text_sdr = NULL;
		ShaderProg *atmosphere_sdr = NULL;
		ShaderProg *dimmed_sky_sdr = NULL;
		ShaderProg *drone_shield_sdr = NULL;
		ShaderProg *point_sprite_heat_sdr = NULL;
        /*---------------------------------------------*/

        /*Particles System maps and configurations*/
        std::vector<NucEmitterConfig> emitter_configs;
        std::vector<NucPhysics*> physics_sims;

        std::map<std::string, unsigned int> emitter_config_idx_by_name;
        std::map<std::string, unsigned int> physics_sim_idx_by_name;
        /*---------------------------------------------*/

        /*Renderers*/
        PointSpriteRenderer *ps_r = NULL;
        PointSpriteOrthoRenderer *ps_ortho_r = NULL;
		PointSpriteRenderer *ps_heat_renderer = NULL;
        /*---------------------------------------------*/

        /*Texture set (manages textures)*/
        goatgfx::TextureSet *texset = NULL;
        /*----------------------------------*/

        /*Managers and Handlers*/
        NucManager *nuc_manager = NULL;
        ScreenManager *scrn_manager = NULL;
		AudioManager *audio_manager = NULL;

        EventHandler *event_handler = NULL;
        /*----------------------------------*/

        /*The currently active stage.*/
        Stage *active_stage = NULL;
        /*----------------------------------*/

        /*Program Exit Switch*/
        bool exit_game;
        /*---------------------*/

        /*control flags*/
        bool warp_mouse;
        /*----------------------------------*/

        /*Render target*/
        RenderTarget rtarg;
		RenderTarget rtarg2;
        /*------------------------------------*/

		/*Ovr Manager*/
		OvrManager ovr_manager;
		/*---------------------------------------*/

		/*Ovr initialization flag*/
		bool ovr_initialized;
		/*--------------------------------------*/

		/*HMD fullscreen flag*/
		bool hmd_fullscrn_flag;
		/*---------------------------------------*/

		/*Render pass type enumeration*/
		RenderPassType rndr_pass_type;
		/*----------------------------------------*/

		void generate_rendering_quad()
		{
			rendering_quad = new Mesh;
			Vertex v;
			Face f;

			//1st face
			v.pos = Vector3(-1, -1, 0);
			v.u = 0;
			v.v = 0;
			rendering_quad->add_vertex(v);

			v.pos = Vector3(1, -1, 0);
			v.u = 1.0;
			v.v = 0;
			rendering_quad->add_vertex(v);

			v.pos = Vector3(1, 1, 0);
			v.u = 1.0;
			v.v = 1.0;
			rendering_quad->add_vertex(v);

			//2nd face
			v.pos = Vector3(-1, 1, 0);
			v.u = 0;
			v.v = 1.0;
			rendering_quad->add_vertex(v);

			v.pos = Vector3(-1, -1, 0);
			v.u = 0;
			v.v = 0;
			rendering_quad->add_vertex(v);

			v.pos = Vector3(1, 1, 0);
			v.u = 1.0;
			v.v = 1.0;
			rendering_quad->add_vertex(v);


			int counter = 0;

			for (unsigned int i = 0; i < 2; i++){
				for (unsigned int j = 0; j < 3; j++){
					f.vidx[j] = counter++;
				}
				rendering_quad->add_index(f);
			}
		}

		void update_rendering_quad_tc()
		{
			int fb_width = game::engine::rtarg.get_fb_width();
			int fb_height = game::engine::rtarg.get_fb_height();
			int fb_tex_width = game::engine::rtarg.get_fb_tex_width();
			int fb_tex_height = game::engine::rtarg.get_fb_tex_height();

			float u_scaled = (float)fb_width / (float)fb_tex_width;
			float v_scaled = (float)fb_height / (float)fb_tex_height;
			
			Matrix4x4 tex_mat;
			tex_mat.scale(Vector4(u_scaled, v_scaled, 1.0, 1.0));

			set_texture_matrix(tex_mat);
		}
    }

    namespace utils
    {

        //SphericalCoord-----------------------------------------------------
        SphericalCoord::SphericalCoord()
        {
            r = 0;
            phi = 0;
            theta = 0;
        }

        SphericalCoord::SphericalCoord(float r, float phi, float theta)
        {
            this->r = r;
            this->phi = phi;
            this->theta = theta;
        }

        /*Unprojects a 2D vector. (Moves it to world space) */
        Vector3 unproject(int win_x, int win_y, int win_z, const Matrix4x4 &proj, const Matrix4x4 &view, int *viewport)
        {
            Matrix4x4 pv = proj * view;
            Matrix4x4 inv_pv = pv.inverse();

            float x = (win_x - (float)viewport[0]) / (float)viewport[2] * 2.0 - 1.0;
            float y = (win_y - (float)viewport[1]) / (float)viewport[3] * 2.0 - 1.0;
            float z = 2.0 * win_z - 1.0;

            Vector4 res = Vector4(x, y, z, 1.0);

            res.transform(inv_pv);

            if (res.w == 0)
                return Vector3();

            res.w = 1.0 / res.w;

            return Vector3(res.x * res.w, res.y * res.w, res.z * res.w);
        }

        unsigned int next_pow2(unsigned int num)
        {
            num -= 1;
            num |= num >> 1;
            num |= num >> 2;
            num |= num >> 4;
            num |= num >> 8;
            num |= num >> 16;
            return num + 1;
        }

        SphericalCoord cartesian_to_spherical(const Vector3 &v)
        {
            float r = sqrt(v.x * v.x + v.y * v.y + v.z + v.z);
            float phi = acos(v.y / r);
            float theta = atan(v.y / v.x);

            return SphericalCoord(r,phi,theta);
        }

        //Y-up----
        Vector3 spherical_to_cartesian_y_ref(float vertical_angle, float horizontal_angle, float radius)
        {
			return Vector3(radius * sin(vertical_angle)*sin(horizontal_angle), radius * cos(horizontal_angle),
				radius * cos(vertical_angle)*sin(horizontal_angle));
        }

		Vector3 spherical_to_cartesian_z_ref(float vertical_angle, float horizontal_angle, float radius)
		{
			return Vector3(radius * cos(vertical_angle) * sin(horizontal_angle), radius * sin(vertical_angle),
				-(radius * cos(vertical_angle) * cos(horizontal_angle)));
		}

        void gen_uv_sphere(Mesh *m, float radius, int usub , int vsub , float urange , float vrange)
        {
            Vertex tmp_vert;
            Face tmp_face1,tmp_face2;

            if(usub < 4)
                usub = 4;
            if(vsub < 2)
                vsub = 2;

            int u_verts = usub+1;
            int v_verts = vsub+1;

            float du = urange / (float)(u_verts - 1);
            float dv = vrange / (float)(v_verts - 1);

            float u = 0.0;

            for(int i = 0 ; i < u_verts ; i++){
                float theta = (u * 2.0 * M_PI) * urange;

                float v = 0.0;
                for(int j = 0 ; j < v_verts ; j++){
                    float phi = (v * M_PI) * vrange;
                    tmp_vert.pos = spherical_to_cartesian_y_ref(theta, phi) * radius;
                    tmp_vert.normal = tmp_vert.pos;
                    tmp_vert.tangent = (spherical_to_cartesian_y_ref(theta + 1.0, M_PI / 2.0) -
                                        spherical_to_cartesian_y_ref(theta - 1.0, M_PI / 2.0)).normalized();
                    tmp_vert.u = u * urange;
                    tmp_vert.v = v * vrange;

                    if(i < usub && j < vsub){
                        int idx = i * v_verts+j;

                        tmp_face1.vidx[0] = idx;
                        tmp_face1.vidx[1] = idx + 1;
                        tmp_face1.vidx[2] = idx + v_verts + 1;

                        tmp_face2.vidx[0] = idx;
                        tmp_face2.vidx[1] = idx + v_verts + 1;
                        tmp_face2.vidx[2] = idx + v_verts;
                    }

                    m->add_vertex(tmp_vert);
                    m->add_index(tmp_face1);
                    m->add_index(tmp_face2);

                    v += dv;
                }
                u += du;
            }
        }

        void gen_cube(Mesh *m, float x_size, float y_size, float z_size, const Vector3 &pivot_loc)
        {
            Vertex v;
            Face f;
            Vector3 norm;

            //Front
            norm = Vector3(0, 0, 1);

            v.pos = Vector3(x_size, -y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, -y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, -y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            //Back
            norm = Vector3(0, 0, -1);

            v.pos = Vector3(-x_size, -y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, -y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, -y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            //Right
            norm = Vector3(1, 0, 0);

            v.pos = Vector3(x_size, -y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, -y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, -y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            //Left
            norm = Vector3(-1, 0, 0);

            v.pos = Vector3(-x_size, -y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, -y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, -y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            //Bottom
            norm = Vector3(0, -1, 0);

            v.pos = Vector3(-x_size, -y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, -y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, -y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, -y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, -y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, -y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            //Top
            norm = Vector3(0, 1, 0);

            v.pos = Vector3(x_size, y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(x_size, y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, y_size, -z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            v.pos = Vector3(-x_size, y_size, z_size) + pivot_loc;
            v.normal = norm;
            m->add_vertex(v);

            int counter = 0;

            for(unsigned int i = 0 ; i < 12 ; i++){
                for(unsigned int j = 0 ; j < 3 ; j++){
                    f.vidx[j] = counter++;
                }
                m->add_index(f);
            }
        }

		EnemyTemplate *get_enemy_template_by_name(const std::string &name)
		{
			unsigned int idx = assets::enemy_idx_by_name[name];
			return assets::enemy_templates[idx];
		}

		AudioSample *get_audio_sample_by_name(const std::string &name)
		{
			unsigned int idx = assets::audio_sample_idx_by_name[name];
			return assets::audio_samples[idx];
		}

		void create_scene_psys_emitters(Object *obj)
		{
			std::string name = obj->get_name();

			unsigned int pos = name.find_first_of('_');

			std::string substr1 = name.substr(0, pos);

			if (substr1.compare("NUC") == 0){
				if (!game::engine::emitter_configs.empty() && !game::engine::physics_sims.empty()){
					NucEmitter *em;
					NucPhysics *ph;

					unsigned int pos2 = name.find_first_of('$');
					std::string substr2 = name.substr(pos + 1, pos2 - pos - 1);

					unsigned int fullstop = substr2.find('.');

					std::string emitter_name = substr2.substr(0, fullstop);
					std::string physics_name = substr2.substr(fullstop + 1);

					em = new NucEmitter;

					unsigned int em_idx = game::engine::emitter_config_idx_by_name[emitter_name];
					unsigned int ph_idx = game::engine::physics_sim_idx_by_name[physics_name];

					em->set_emitter_config(game::engine::emitter_configs[em_idx]);

					ph = game::engine::physics_sims[ph_idx];

					em->set_physics_simulator(ph);
					em->set_renderer(game::engine::ps_r);
					em->init();

					obj->add_child(em);
					game::engine::nuc_manager->add_emitter(em);
				}
			}

			for (unsigned int i = 0; i < obj->get_children_count(); i++){
				Object *child = (Object*)obj->get_child(i);
				create_scene_psys_emitters(child);
			}
		}

        void display_to_rendering_quad()
        {
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Matrix4x4 view = get_view_matrix();
            Matrix4x4 proj = get_projection_matrix();

            set_world_matrix(Matrix4x4());
            set_view_matrix(Matrix4x4());
            set_projection_matrix(Matrix4x4());

            glViewport(0, 0, game::engine::rtarg.get_fb_width() / game::options::ss_factor,
                game::engine::rtarg.get_fb_height() / game::options::ss_factor);

            unsigned int tex = game::engine::rtarg.get_color_attachment();
            glBindTexture(GL_TEXTURE_2D, tex);
            set_unistate("st_fbo_color_tex", 0);

            game::engine::fb_color_sdr->bind();
            game::engine::rendering_quad->render();
            game::engine::main_sdr->bind();

            glBindTexture(GL_TEXTURE_2D, 0);

            set_view_matrix(view);
            set_projection_matrix(proj);
        }

    }//utils namespace

	/*game initialization*/
	bool init_shaders()
	{
		sdrman_init();

        engine::main_sdr = get_sdrprog("main_vs.glsl", "main_fs.glsl");
        if (!engine::main_sdr){
			std::cerr << "Failed to create the main shader!" << std::endl;
			return false;
		}
        bind_attributes(engine::main_sdr);


		engine::main_no_shad_sdr = get_sdrprog("main_no_shad_vs.glsl", "main_no_shad_fs.glsl");
		if (!engine::main_no_shad_sdr){
			std::cerr << "Failed to create the Main shader with no shadows!" << std::endl;
			return false;
		}
		bind_attributes(engine::main_no_shad_sdr);

		engine::holomap_sdr = get_sdrprog("holomap_vs.glsl", "holomap_fs.glsl");
		if (!engine::holomap_sdr){
			std::cerr << "Failed to create the Holomap shader with no shadows!" << std::endl;
			return false;
		}
		bind_attributes(engine::holomap_sdr);

		engine::no_lighting_sdr = get_sdrprog("no_lighting_vs.glsl", "no_lighting_fs.glsl");
		if (!engine::no_lighting_sdr){
			std::cerr << "Failed to create the Holomap shader with no shadows!" << std::endl;
			return false;
		}
		bind_attributes(engine::no_lighting_sdr);

		engine::crosshair_sdr = get_sdrprog("crosshair_vs.glsl", "crosshair_fs.glsl");
		if (!engine::crosshair_sdr){
			std::cerr << "Failed to create the Holomap shader with no shadows!" << std::endl;
			return false;
		}
		bind_attributes(engine::crosshair_sdr);

		engine::holomap_4dnoise_sdr = get_sdrprog("holomap_4dsnoise_vs.glsl", "holomap_4dsnoise_fs.glsl");
		if (!engine::holomap_4dnoise_sdr){
			std::cerr << "Failed to create the Holomap shader with no shadows!" << std::endl;
			return false;
		}
		bind_attributes(engine::holomap_4dnoise_sdr);

		engine::font_sdr = get_sdrprog("font_vs.glsl", "font_fs.glsl");
		if (!engine::font_sdr){
			std::cerr << "Failed to create the Holomap shader with no shadows!" << std::endl;
			return false;
		}
		bind_attributes(engine::font_sdr);

		engine::flashing_text_sdr = get_sdrprog("flashing_text_vs.glsl", "flashing_text_fs.glsl");
		if (!engine::flashing_text_sdr){
			std::cerr << "Failed to create the Holomap shader with no shadows!" << std::endl;
			return false;
		}
		bind_attributes(engine::flashing_text_sdr);

        engine::cubemap_sdr = get_sdrprog("sky_vs.glsl", "sky_fs.glsl");
        if(!engine::cubemap_sdr){
            std::cerr<<"Failed to create the cubemap shader!"<<std::endl;
            return false;
        }
        bind_attributes(engine::cubemap_sdr);

        engine::fb_color_sdr = get_sdrprog("color_tex_vs.glsl", "color_tex_fs.glsl");
        if(!engine::fb_color_sdr){
            std::cerr<<"Failed to creat the fbo color attachment shader!"<<std::endl;
            return false;
        }
        bind_attributes(engine::fb_color_sdr);

        engine::fb_color_sdr_fxaa = get_sdrprog("color_tex_vs.glsl", "color_tex_fxaa_fs.glsl");
        if(!engine::fb_color_sdr_fxaa){
            std::cerr<<"Failed to create the fbo FXAA filter shader!"<<std::endl;
            return false;
        }
        bind_attributes(engine::fb_color_sdr_fxaa);

        engine::point_sprite_sdr = get_sdrprog("point_sprite_vs.glsl", "point_sprite_fs.glsl");
        if(!engine::point_sprite_sdr){
            std::cerr << "Failed to create the point sprite shader!" << std::endl;
            return false;
        }

		engine::fresnel_sdr = get_sdrprog("fresnel_vs.glsl", "fresnel_fs.glsl");
		if (!engine::fresnel_sdr){
			std::cerr << "Failed to create the fresnel reflection/refraction shader!" << std::endl;
			return false;
		}
		bind_attributes(engine::fresnel_sdr);

		engine::depth_pass_sdr = get_sdrprog("depth_pass_vs.glsl", "depth_pass_fs.glsl");
		if (!engine::depth_pass_sdr){
			std::cerr << "Faled to create the depth pass shader!" << std::endl;
			return false;
		}
		bind_attributes(engine::depth_pass_sdr);

		engine::depth_dbg_sdr = get_sdrprog("depth_dbg_vs.glsl", "depth_dbg_fs.glsl");
		if (!engine::depth_dbg_sdr){
			std::cerr << "Failed to create the depth debug shader!" << std::endl;
			return false;
		}
		bind_attributes(engine::depth_dbg_sdr);

		engine::atmosphere_sdr = get_sdrprog("atmosphere_vs.glsl", "atmosphere_fs.glsl");
		if (!engine::atmosphere_sdr){
			std::cerr << "Failed to create the atmosphere shader!" << std::endl;
			return false;
		}
		bind_attributes(engine::atmosphere_sdr);

		engine::drone_shield_sdr = get_sdrprog("drone_shield_vs.glsl", "drone_shield_fs.glsl");
		if (!engine::drone_shield_sdr){
			std::cerr << "Failed to create the drone shield shader!" << std::endl;
			return false;
		}
		bind_attributes(engine::drone_shield_sdr);

		engine::dimmed_sky_sdr = get_sdrprog("dimmed_sky_vs.glsl", "dimmed_sky_fs.glsl");
		if (!engine::dimmed_sky_sdr){
			std::cerr << "Failed to create the dimmed sky shader!" << std::endl;
			return false;
		}
		bind_attributes(engine::dimmed_sky_sdr);

		engine::point_sprite_heat_sdr = get_sdrprog("point_sprite_heat_vs.glsl", "point_sprite_heat_fs.glsl");
		if (!engine::point_sprite_heat_sdr){
			std::cerr << "Failed to create the heat point sprite shader!" << std::endl;
			return false;
		}
		bind_attributes(engine::point_sprite_heat_sdr);

		return true;
	}

	bool init_managers()
	{
        engine::scrn_manager = new ScreenManager;
        if (!engine::scrn_manager){
			std::cerr << "Could not allocate the screen manager!" << std::endl;
			return false;
		}

        engine::nuc_manager = new NucManager;
        if (!engine::nuc_manager){
			std::cerr << "Could not allocate the Nucleus manager!" << std::endl;
			return false;
		}

		engine::audio_manager = new AudioManager;
		if (!engine::audio_manager){
			std::cerr << "Could not allocate the Audio manager!" << std::endl;
			return false;
		}

		return true;
	}

	bool init_handlers()
	{
        engine::event_handler = new EventHandler;
        if (!engine::event_handler){
			std::cerr << "Could not allocate the Event Handler!" << std::endl;
			return false;
		}

		return true;
	}

	bool init_screens()
	{
		engine::starting_scrn = new StartingScreen;
		if (!engine::starting_scrn){
			std::cerr << "Could nto allocate the starting screen!" << std::endl;
			return false;
		}

        engine::game_scrn = new GameScreen;
        if (!engine::game_scrn){
			std::cerr << "Could not allocate the game screen!" << std::endl;
			return false;
		}

		engine::game_over_scrn = new GameOverScreen;
		if (!engine::game_over_scrn){
			std::cerr << "Could not allocate the game over screen!" << std::endl;
			return false;
		}

        engine::credits_scrn = new CreditsScreen;
        if(!engine::credits_scrn) {
            std::cerr << "Could not allocate the credits screen!" << std::endl;
        }

		return true;
	}

	bool init_dummy_textures()
	{
        if (!(assets::diff_dummy = engine::texset->get("diff_dummy_16.jpg"))){
			std::cerr << "Diffuse Dummy Texture could not be loaded!" << std::endl;
			return false;
		}

        if (!(assets::spec_dummy = engine::texset->get("spec_dummy_16.jpg"))){
			std::cerr << "Specular Dummy Texture could not be loaded!" << std::endl;
			return false;
		}

        if (!(assets::norm_dummy = engine::texset->get("norm_dummy_16.png"))){
			std::cerr << "Normalmap Dummy Texture could not be loaded!" << std::endl;
			return false;
		}

		return true;
	}

	bool pre_load_init()
	{
		add_standart_state_uniforms();
		add_datapaths();

		options::ovr_enabled = false;

		engine::texset = new goatgfx::TextureSet;


		options::ss_factor = 2.0;

		engine::rtarg.set_fb_width(options::win_width * options::ss_factor);
		engine::rtarg.set_fb_height(options::win_height * options::ss_factor);
		engine::rtarg.set_shad_tex_width(2048);
		engine::rtarg.set_shad_tex_height(2048);
		if (!engine::rtarg.create(RT_ALL)){
			std::cerr << "Failed to create the render target!" << std::endl;
			return false;
		}

		engine::rtarg2.set_shad_tex_width(4096);
		engine::rtarg2.set_shad_tex_height(4096);
		if (!engine::rtarg2.create(RT_SHADOW_MASK)){
			std::cerr << "Failed to create the 2nd render target!" << std::endl;
		}

		assets::font = dtx_open_font("freeagent.ttf", 1);

		if (!init_dummy_textures()){
			std::cerr << "Failed to initialize the dummy textures!" << std::endl;
			return false;
		}

		if (!init_shaders()){
			std::cerr << "Failed to initialize the shaders!" << std::endl;
			return false;
		}

		if (!init_managers()){
			std::cerr << "Failed to initialize the managers!" << std::endl;
			return false;
		}

		if (!init_handlers()){
			std::cerr << "Failed to initialize the handlers!" << std::endl;
			return false;
		}

		if (!init_screens()){
			std::cerr << "Failed to initialize the screens!" << std::endl;
			return false;
		}

		engine::current_sdr = engine::main_sdr;

		engine::ps_r = new PointSpriteRenderer;
		engine::ps_r->set_shader_program(engine::point_sprite_sdr);
		engine::ps_r->bind_attributes();
		engine::ps_r->init();

		engine::ps_heat_renderer = new PointSpriteRenderer;
		engine::ps_heat_renderer->set_shader_program(engine::point_sprite_heat_sdr);
		engine::ps_heat_renderer->bind_attributes();
		engine::ps_heat_renderer->init();

		engine::warp_mouse = true;

		engine::scrn_manager->push_screen(engine::starting_scrn);

		init_audio();

		game::options::fullscreen = true;

		return true;
	}

	bool post_load_init()
	{
		engine::scrn_manager->init_active_scrn();
		engine::scrn_manager->start_active_scrn();

		engine::generate_rendering_quad();
		game::engine::update_rendering_quad_tc();

		return true;
	}

    bool load(const char *fname)
    {
        NCF ncf;

        ncf.set_source(fname);

        if(ncf.parse() != 0){
            std::cerr << "Could not parse " << fname << std::endl;
            return false;
        }

        /*Particle system emitter and physics configuration loading*/
        NCF *node = NULL;
        node = ncf.get_group_by_name("psys_config_fnames");
        if(!node){
            std::cerr << "Could not parse: psys_config_fnames." << std::endl;
            return false;
        }

        for(unsigned int i = 0 ; i < node->count_properties() ; i++){
            std::string name = node->get_property_by_index(i);

            if(!load_emitter_configs(name.c_str())){
                std::cerr << "Could not load the emitter configs." << std::endl;
                return false;
            }
        }

        node = ncf.get_group_by_name("psys_physics_config_fnames");
        if(!node){
            std::cerr << "Could not parse: psys_physics_config_fnames." << std::endl;
            return false;
        }

        for(unsigned int i = 0 ; i < node->count_properties() ; i++){
            std::string name = node->get_property_by_index(i);

            if(!load_physics_sim_configs(name.c_str())){
                std::cerr << "Coud not load the physics simulators." << std::endl;
                return false;
            }
        }
        /*--------------------------------------------------------------------------------*/

        /*Enemy template loading.*/
        node = ncf.get_group_by_name("enemy_tmpl_fnames");
        if(!node){
            std::cerr << "Could not parse: enemy_tmpl_fanmes." << std::endl;
            return false;
        }

        for(unsigned int i = 0 ; i < node->count_properties() ; i++){
            std::string name = node->get_property_name_by_index(i);
            assets::enemy_idx_by_name[name] = i;

            std::string path = node->get_property_by_name(name.c_str());

            if(!load_enemy_tmpl(path.c_str())){
                std::cerr << "Could not load: "<< name << std::endl;
                return false;
            }
        }
        /*---------------------------------------------------------------------------------*/

        /*Enemy animation loading*/
        node = ncf.get_group_by_name("enemy_animations");
        if(!node){
            std::cerr << "Could not parse the enemy animation files!" << std::endl;
            return false;
        }

        for(unsigned int i = 0 ; i < node->count_groups() ; i++){
            NCF *grp = node->get_group_by_index(i);
            std::string anim_name = grp->get_name();
            std::string path = grp->get_property_by_name("path");
            std::string loop = grp->get_property_by_name("looping");

            bool loop_state;

            if(loop.compare("true") == 0){
                loop_state = true;
            }

            if(loop.compare("false") == 0){
                loop_state = false;
            }

            for(unsigned int j = 0 ; j < assets::enemy_templates.size() ; j++){
                EnemyTemplate *tmpl = assets::enemy_templates[j];
                tmpl->load_animation(path.c_str(), anim_name.c_str(), loop_state);
            }
        }
        /*----------------------------------------------------------------------------------*/

		/*Load enemy template attribute config files and initialize the templates' values.*/
		node = ncf.get_group_by_name("enemy_config_fnames");
		if (!node){
			std::cerr << "Could not parse the enemy config files!" << std::endl;
			return false;
		}

		for (unsigned int i = 0; i < node->count_properties(); i++){
			std::string name = (std::string)node->get_property_name_by_index(i);
			std::string path = (std::string)node->get_property_by_name(name.c_str());
			unsigned int idx = assets::enemy_idx_by_name[name];
            EnemyTemplate *tmpl = assets::enemy_templates[idx];

			ConfigFile cfg;
			if (!cfg.open(path.c_str())){
				std::cerr << "Could not load the template attributes!" << std::endl;
				return false;
			}
			tmpl->set_name(std::string("enemy_").append(name));
			tmpl->set_hp(cfg.get_int("attributes.hp"));
			tmpl->set_speed(cfg.get_num("attributes.speed"));
			tmpl->set_dmg(cfg.get_int("attributes.damage"));
			tmpl->init_b_obj();
		}
		/*----------------------------------------------------------------------------------*/

		/*Load Audio Samples*/
		node = ncf.get_group_by_name("audio_samples");
		if (!node){
			std::cerr << "Could not parse the audio samples!" << std::endl;
			return false;
		}

		for (unsigned int i = 0; i < node->count_properties(); i++){
			std::string name = (std::string)node->get_property_name_by_index(i);
			std::string path = (std::string)node->get_property_by_name(name.c_str());

			AudioSample *sample = new AudioSample;
			if (!sample->load(path.c_str())){
				std::cout << "Could not load " << path << std::endl;
				return false;
			}

			assets::audio_samples.push_back(sample);
			assets::audio_sample_idx_by_name[name] = i;
		}
		/*----------------------------------------------------------------------------------*/

		/*Load Starting Screen Scenes*/
		node = ncf.get_group_by_name("starting_screen_scenes");
		if (!node){
			std::cerr << "Could not parse the starting screen scenes!" << std::endl;
			return false;
		}

		for (unsigned int i = 0; i < node->count_properties(); i++){
			std::string path = (std::string)node->get_property_by_index(i);

			Scene *sc = NULL;
			sc = new Scene;
			if (!sc->load(path.c_str())){
				std::cerr << "Failed to load the scene:" << path << std::endl;
				return false;
			}

			if (!sc->load_animation(path.c_str(), "scene_anim", true)){
				std::cerr << "Failed to load the scene animation! Path:" << path << std::endl;
			}
			else{
				sc->start_animation(1, 0);
			}

			std::string sc_name = node->get_property_name_by_index(i);
			StartingScreen *scrn = (StartingScreen*)game::engine::starting_scrn;
			scrn->map_scene_by_name(sc_name, sc);
			scrn->add_scene(sc);
		}
		/*-----------------------------------------------------------------------------------*/

		/*Load the Game Over Screen Scenes*/
		node = ncf.get_group_by_name("game_over_screen_scenes");
		if (!node){
			std::cerr << "Could not parse the game over screen scenes!" << std::endl;
			return false;
		}

		for (unsigned int i = 0; i < node->count_properties(); i++){
			std::string path = (std::string)node->get_property_by_index(i);

			Scene *sc = NULL;
			sc = new Scene;
			if (!sc->load(path.c_str())){
				std::cerr << "Failed to load the scene:" << path << std::endl;
				return false;
			}

			if (!sc->load_animation(path.c_str(), "scene_anim", true)){
				std::cerr << "Failed to load the scene animation! Path:" << path << std::endl;
			}
			else{
				sc->start_animation(1, 0);
			}

			std::string sc_name = node->get_property_name_by_index(i);
			GameOverScreen *scrn = (GameOverScreen*)game::engine::game_over_scrn;
			scrn->map_scene_by_name(sc_name, sc);
			scrn->add_scene(sc);
		}
		/*------------------------------------------------------------------------------------*/

		/*Load Credits Screen Scenes*/
		node = ncf.get_group_by_name("credits_screen_scenes");
		if (!node){
			std::cerr << "Could not parse the credits screen scenes!" << std::endl;
			return false;
		}

		for (unsigned int i = 0; i < node->count_properties(); i++){
			std::string path = (std::string)node->get_property_by_index(i);

			Scene *sc = NULL;
			sc = new Scene;
			if (!sc->load(path.c_str())){
				std::cerr << "Failed to load the scene:" << path << std::endl;
				return false;
			}

			if (!sc->load_animation(path.c_str(), "scene_anim", true)){
				std::cerr << "Failed to load the scene animation! Path:" << path << std::endl;
			}
			else{
				sc->start_animation(1, 0);
			}

			std::string sc_name = node->get_property_name_by_index(i);
			CreditsScreen *scrn = (CreditsScreen*)game::engine::credits_scrn;
			scrn->map_scene_by_name(sc_name, sc);
			scrn->add_scene(sc);
		}
		/*------------------------------------------------------------------------------------*/

        engine::active_stage = new Stage;
        if(!engine::active_stage->load("data/config/stage1.config")){
            std::cout<<"Failed to load the stage! Aborting!"<<std::endl;
            return false;
        }
        

        return true;
    }

    void cleanup()
    {
        cleanup_stages();
        cleanup_assets();
        cleanup_screens();
        cleanup_managers();
        cleanup_handlers();
        cleanup_physics_sims();
        cleanup_shaders();

		delete engine::rendering_quad;

        //make this a seperate function.
        if(!assets::audio_samples.empty()) {
            std::vector<AudioSample*>::iterator it = assets::audio_samples.begin();

            while(it != assets::audio_samples.end()) {
                delete (*it);
                it = assets::audio_samples.erase(it);
            }
        }

        delete engine::texset;
		delete engine::ps_r;
		delete engine::ps_ortho_r;
		delete engine::ps_heat_renderer;

		if (engine::ovr_initialized)
			engine::ovr_manager.destroy_ovr_data();
		
		destroy_audio();
    }

    void cleanup_assets()
    {
        if(!assets::enemy_templates.empty()){
            for(unsigned int i = 0 ; i < assets::enemy_templates.size() ; i++){
                delete assets::enemy_templates[i];
            }
        }

        if(!assets::enemy_idx_by_name.empty())
            assets::enemy_idx_by_name.clear();
    }

    void cleanup_screens()
    {
        delete engine::game_scrn;
        delete engine::logo_scrn;
		delete engine::starting_scrn;
		delete engine::credits_scrn;
    }

    void cleanup_stages()
    {
        delete engine::active_stage;
    }

    void cleanup_managers()
    {
		delete engine::nuc_manager;
        delete engine::scrn_manager;
    }

    void cleanup_handlers()
    {
        if(engine::event_handler)
            delete engine::event_handler;
    }

    void cleanup_physics_sims()
    {
        if(!engine::physics_sims.empty()){
            for(unsigned int i = 0; i < engine::physics_sims.size() ; i++){
                delete engine::physics_sims[i];
            }
        }

        if(!engine::physics_sim_idx_by_name.empty())
            engine::physics_sim_idx_by_name.clear();
    }

    void cleanup_shaders()
    {
		delete engine::main_sdr;
        delete engine::cubemap_sdr;
		delete engine::shadow_pass_sdr;
        delete engine::point_sprite_sdr;
        delete engine::fb_color_sdr;
        delete engine::fb_color_sdr_fxaa;
        delete engine::fresnel_sdr;
        delete engine::depth_pass_sdr;
        delete engine::depth_dbg_sdr;
        delete engine::main_no_shad_sdr;
        delete engine::holomap_sdr;
        delete engine::holomap_4dnoise_sdr;
        delete engine::font_sdr;
        delete engine::no_lighting_sdr;
        delete engine::crosshair_sdr;
        delete engine::flashing_text_sdr;
        delete engine::atmosphere_sdr;
		delete engine::dimmed_sky_sdr;
		delete engine::drone_shield_sdr;

        sdrman_destroy();
    }
}


/*Statics---------------------------------------------------------*/

static bool load_enemy_tmpl(const char *fname)
{
	EnemyTemplate *tmpl = new EnemyTemplate;
	if (!tmpl->load(fname)) {
		std::cout << "Could not load " << fname << ". Abroting!!!" << std::endl;
		return false;
	}
    game::assets::enemy_templates.push_back(tmpl);
	return true;
}

static bool load_emitter_configs(const char *fname)
{
	ConfigFile cfg;
	NucEmitterConfig em_cfg;

	if (!cfg.open(fname)) {
		std::cout << "Could not parse " << fname << ". Aborting!!!" << std::endl;
		return false;
	}

	em_cfg.spawn_rate = cfg.get_num("emitter_variables.spawn_rate", 1);
	em_cfg.lifespan = cfg.get_num("emitter_variables.lifespan", 3);
	em_cfg.max_particles = cfg.get_int("emitter_variables.max_particles", 1);
	em_cfg.spawn_radius = cfg.get_num("emitter_variables.spawn_radius", 1);
	em_cfg.tex_path = cfg.get_str("emitter_variables.tex_path",
		"data/textures/dummies/diff_dummy_16.jpg");
	em_cfg.start_color = cfg.get_vec("emitter_variables.start_color", Vector4(1, 1, 1, 1));
	em_cfg.end_color = cfg.get_vec("emitter_variables.end_color", Vector4(1, 1, 1, 1));
	em_cfg.size = cfg.get_num("emitter_variables.size", 200);
	em_cfg.emission_duration = cfg.get_int("emitter_variables.emission_duration");

	std::string tmp = fname;

	unsigned int end = tmp.find_last_of('_');
	tmp = tmp.substr(0, end);

    game::engine::emitter_configs.push_back(em_cfg);
    game::engine::emitter_config_idx_by_name[tmp] = game::engine::emitter_configs.size() - 1;

	return true;
}

static bool load_physics_sim_configs(const char *fname)
{
	ConfigFile cfg;

	if (!cfg.open(fname)) {
		std::cout << "Could not parse " << fname << ". Aborting!!!" << std::endl;
		return false;
	}

	NucPhysics *ph = new NucPhysics;

	Vector4 vel = cfg.get_vec("physics_variables.initial_velocity");
	ph->set_init_velocity(Vector3(vel.x, vel.y, vel.z));

	ph->set_velocity_range(cfg.get_num("physics_variables.velocity_range", 0));

	Vector4 ext_f = cfg.get_vec("physics_variables.external_force");
	ph->set_external_force(Vector3(ext_f.x, ext_f.y, ext_f.z));

	std::string tmp = fname;

	unsigned int end = tmp.find_last_of('_');
	tmp = tmp.substr(0, end);

    game::engine::physics_sims.push_back(ph);
    game::engine::physics_sim_idx_by_name[tmp] = game::engine::physics_sims.size() - 1;

	return true;
}

static void add_standart_state_uniforms()
{
	add_unistate("st_world_matrix", ST_MATRIX4);
	add_unistate("st_world_matrix_transpose", ST_MATRIX4);
	add_unistate("st_world_matrix3", ST_MATRIX3);

	add_unistate("st_view_matrix", ST_MATRIX4);
	add_unistate("st_view_matrix_transpose", ST_MATRIX4);
	add_unistate("st_view_matrix3", ST_MATRIX3);

	add_unistate("st_proj_matrix", ST_MATRIX4);

	add_unistate("st_tex_matrix", ST_MATRIX4);
	add_unistate("st_shadow_matrix", ST_MATRIX4);

	add_unistate("st_time", ST_FLOAT);

	add_unistate("st_light_pos", ST_FLOAT3);
	add_unistate("st_light_color_diff", ST_FLOAT3);
	add_unistate("st_spot_direction", ST_FLOAT3);
	add_unistate("st_light_color_ambient", ST_FLOAT4);
	add_unistate("st_light_intensity", ST_FLOAT);
	add_unistate("st_light_radius", ST_FLOAT);
	add_unistate("st_light_att_constant", ST_FLOAT);
	add_unistate("st_light_att_linear", ST_FLOAT);
	add_unistate("st_light_att_quadratic", ST_FLOAT);

	add_unistate("st_fog_start", ST_FLOAT);
	add_unistate("st_fog_end", ST_FLOAT);
	add_unistate("st_fog_color", ST_FLOAT4);

	add_unistate("st_mtl_diffuse", ST_FLOAT4);
	add_unistate("st_mtl_specular", ST_FLOAT4);
	add_unistate("st_mtl_shininess", ST_FLOAT);
	add_unistate("st_mtl_alpha", ST_FLOAT);
	add_unistate("st_diff_tex", ST_INT);
	add_unistate("st_spec_tex", ST_INT);
	add_unistate("st_norm_map_tex", ST_INT);
	add_unistate("st_shadow2D_tex", ST_INT);
    add_unistate("st_fbo_color_tex", ST_INT);
    add_unistate("st_ss_factor", ST_FLOAT);
	add_unistate("st_screen_size", ST_FLOAT2);
	add_unistate("st_scaled_uv_values", ST_FLOAT2);
}

static void bind_attributes(ShaderProg *sdr)
{
	sdr->set_attrib_location("attr_vertex", 0);
	sdr->set_attrib_location("attr_normal", 1);
	sdr->set_attrib_location("attr_tangent", 2);
	sdr->set_attrib_location("attr_uv", 3);
}

static void add_datapaths()
{
	add_data_path("sdr");
    add_data_path("data/config");
    add_data_path("data/models");
    add_data_path("data/textures/spaceships");
	add_data_path("data/textures/planets");
	add_data_path("data/textures/cubemaps");
	add_data_path("data/textures/dummies");
	add_data_path("data/textures/particles");
	add_data_path("data/config");
	add_data_path("data/audio");
	add_data_path(".");
}

/*----------------------------------------------------------------*/

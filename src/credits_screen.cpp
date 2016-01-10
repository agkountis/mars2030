#include "credits_screen.h"
#include "game.h"


CreditsScreen::CreditsScreen()
{
    lt = new Light;
    lt->set_position(Vector3(70000, 70000, -70000));
    lt->set_att_constant(1.0);
    //lt->set_att_linear(0.000001);
    lt->set_att_linear(0.0);
    lt->set_att_quadratic(0);
    lt->set_color_diff(1, 1, 1);

    Mesh *m = NULL;
    m = new Mesh;
    game::utils::gen_uv_sphere(m, 1, 10, 10, 1, 1);
    sky.set_mesh(new Mesh(*m));
    sky.set_mesh_ownership(true);
    delete m;

    Material *mat = sky.get_material();
    mat->textures[TEX_DIFFUSE] = game::engine::texset->get("data/textures/cubemaps/purple_spacebox.png");

	playlist.files.push_back("data/audio/engines_of_war.ogg");
	playlist.files.push_back("data/audio/rogue_poets.ogg");

    duration = 11000;
    skip = false;
}

CreditsScreen::~CreditsScreen()
{
    std::list<Scene*>::iterator it = scenes.begin();

    while(it != scenes.end()) {
        delete (*it);
        it = scenes.erase(it);
    }

    if(lt)
        delete lt;
}

/*Private Functions*/
void CreditsScreen::render_default()
{
    lt->set_light();
    set_unistate("st_light_intensity", (float)1.0);

    game::engine::rtarg.bind(RT_COLOR_AND_DEPTH);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    game::engine::current_sdr = game::engine::dimmed_sky_sdr;
    glDisable(GL_DEPTH_TEST);
    sky.render(RENDER_OBJ);
    glEnable(GL_DEPTH_TEST);

    game::engine::current_sdr = game::engine::main_no_shad_sdr;

	view.reset_identity();
    set_view_matrix(view);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (!strings.empty())
		credits_txt->render(get_msec());

	std::list<Scene*>::iterator it = scenes.begin();
	while (it != scenes.end()) {
		(*it)->render(RENDER_OBJ, get_msec());
		it++;
	}
    glDisable(GL_BLEND);

	game::engine::nuc_manager->update(dt, get_msec());
	game::engine::nuc_manager->render(RENDER_PSYS, get_msec());

    game::engine::rtarg.unbind();
    game::utils::display_to_rendering_quad();
}

void CreditsScreen::render_ovr()
{
	static bool is_visible = true;

	if (!game::engine::ovr_initialized){
		std::cout << "Ovr HMD not initialized!" << std::endl;
		return;
	}

	lt->set_light();

	set_unistate("st_light_intensity", (float)1.0);

	if (is_visible) {

		game::engine::ovr_manager.calculate_eye_poses();

		game::engine::ovr_manager.begin_ovr_frame();

		//Update the particles.
		game::engine::nuc_manager->update(dt, get_msec());

		game::engine::rndr_pass_type = RNDR_PASS_COLOR;

		for (int eye = 0; eye < 2; eye++) {

			OvrTransformationData xform_data = game::engine::ovr_manager.get_ovr_transformation_data_per_eye(eye);

			glViewport(xform_data.viewport.position.x, xform_data.viewport.position.y, xform_data.viewport.size.x, xform_data.viewport.size.y);

			//Send the Viewport size to the shader.
			set_unistate("st_screen_size", Vector2(xform_data.viewport.size.x, xform_data.viewport.size.y));

			set_projection_matrix(xform_data.projection);

			view.reset_identity();

			//Send the View matrix to the shader.
			set_view_matrix(xform_data.view * view);

			/*Render calls*/
			game::engine::current_sdr = game::engine::dimmed_sky_sdr;
			glDisable(GL_DEPTH_TEST);
			sky.render(RENDER_OBJ);
			glEnable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			game::engine::current_sdr = game::engine::main_no_shad_sdr;
			std::list<Scene*>::iterator it = scenes.begin();

			while (it != scenes.end()) {
			(*it)->render(RENDER_OBJ, get_msec());
			it++;
			}

			if (!strings.empty()) {
				glDisable(GL_CULL_FACE);
				credits_txt->render(get_msec());
				glEnable(GL_CULL_FACE);
			}

			glDisable(GL_BLEND);

			game::engine::nuc_manager->render(RENDER_PSYS, get_msec());
			/*-----------------------------------------------------------------------------------------*/
		}
	}

	is_visible = game::engine::ovr_manager.submit_ovr_frame();

	game::engine::ovr_manager.draw_ovr_mirror_texture(game::options::win_width, game::options::win_height);
}
/*--------------------------------------------------------------------*/

bool CreditsScreen::init()
{
    BaseScreen::init();

    this->stop();
    this->reset();
    this->start();

    /*Emmiter initialization*/
    em = new NucEmitter;
    NucEmitterConfig em_cfg;
    em_cfg.size = 0.7;
    em_cfg.spawn_rate = 1200;
    em_cfg.spawn_radius = 100;
    em_cfg.tex_path = "data/textures/particles/star.jpg";
    em_cfg.lifespan = 4;
    em_cfg.start_color = Vector4(1.0, 1.0, 1.0, 1.0);
    em_cfg.end_color = Vector4(1.0, 1.0, 1.0, 0.2);
    em_cfg.max_particles = 4900;

    em->set_emitter_config(em_cfg);

    NucPhysics *physics = new NucPhysics;
    physics->set_external_force(Vector3(0.0, 0.0, 400.0));
    physics->set_init_velocity(Vector3(0.0, 0.0, 100.0));
    em->set_physics_simulator(physics);

    em->set_marked_for_death(false);
    em->set_renderer(game::engine::ps_r);
	em->init();

    em->set_position(Vector3(0, 0, -200));

    game::engine::nuc_manager->add_emitter(em);
    /*-------------------------------------------------------------*/

    /*Ship Scene initialization*/
	Scene *sc = NULL;

	sc = credits_scrn_scenes_by_name["base_ship"];

	if (sc) {
		game::utils::create_scene_psys_emitters(sc->get_object(0));
		sc->set_anim_speed(1, 0.3);
	}
    /*--------------------------------------------------------------*/

	/*Credits strings*/
	std::string text;
	text = "THERE IS NO ESCAPE!";
	strings.push(text);
	text = "THINK AGAIN...";
	strings.push(text);
	text = "YOU THINK THIS IS OVER?";
	strings.push(text);
	text = "T.E.I. OF CENTRAL MACEDONIA";
	strings.push(text);
	text = "THESIS DEVELOPED FOR";
	strings.push(text);
	text = "WWW.YAKITOME.COM";
	strings.push(text);
	text = "TEXT TO SPEECH BY";
	strings.push(text);
	text = "GOUNTIS AGGELOS";
	strings.push(text);
	text = "VOICE BY";
	strings.push(text);
	text = "BLACK SUN EMPIRE";
	strings.push(text);
	text = "GEORGE SAVINIDIS";
	strings.push(text);
	text = "NICK TSITLAKIDIS";
	strings.push(text);
	text = "MUSIC";
	strings.push(text);
	text = "AND MANY GREAT ARTISTS\n       FROM THE INTERNET";
	strings.push(text);
	text = "GOUNTIS AGGELOS";
	strings.push(text);
	text = "JOHN TSIOMBIKAS";
	strings.push(text);
	text = "GRAPHICS";
	strings.push(text);
	text = "HERCULES EUTHYMIADES\n   & GOUNTIS AGGELOS";
	strings.push(text);
	text = "SCRIPT";
	strings.push(text);
	text = "GOUNTIS AGGELOS";
	strings.push(text);
	text = "PROGRAMMER";
	strings.push(text);
	text = "MARS2030";
	strings.push(text);
	/*------------------------------------------*/

    /*Credits Text initialization*/ 
    credits_txt = new BitmapText;

	credits_txt = new BitmapText;
	credits_txt->set_font(game::assets::font);
	credits_txt->set_shader_program(game::engine::font_sdr);
	credits_txt->set_shader_attr_indices(0, 3);
	credits_txt->set_font_size(200);
	credits_txt->set_text_alignment(TXT_CENTER_ALIGNMENT);
	credits_txt->set_font_color(Vector4(1, 0, 0, 1));
	credits_txt->set_text(strings.top());
	credits_txt->set_position(Vector3(0, 0, -600));
	credits_txt->set_position(Vector3(0, 0, 30), 16000);
	credits_txt->set_scaling(Vector3(0.0118, 0.0118, 0.0118));
	credits_txt->init();
    /*--------------------------------------------------------------*/

	start_time = get_msec();

    return true;
}

void CreditsScreen::render()
{
    if(game::options::ovr_enabled) {
        render_ovr();
    }
    else {
        render_default();
    }
}

void CreditsScreen::on_mouse_down(int btn, int x, int y)
{
	if (get_msec() > duration) {
		skip = true;
	}
}

void CreditsScreen::on_move(int x, int y)
{

}

void CreditsScreen::on_key_press(unsigned char key, int x, int y)
{

}

void CreditsScreen::reshape(int x,int y)
{
    if (!game::options::ovr_enabled){
        int new_tex_width, new_tex_height;

        game::engine::rtarg.set_fb_width(x * game::options::ss_factor);
        game::engine::rtarg.set_fb_height(y * game::options::ss_factor);

        glViewport(0, 0, game::engine::rtarg.get_fb_width(), game::engine::rtarg.get_fb_height());
        set_unistate("st_screen_size", Vector2(game::engine::rtarg.get_fb_tex_width(), game::engine::rtarg.get_fb_height()));
        float aspect = (float)game::engine::rtarg.get_fb_width() / (float)game::engine::rtarg.get_fb_height();

        projection_mat.set_perspective(DEG_TO_RAD(90.0), aspect, 0.1, 40000);
        set_projection_matrix(projection_mat);

        int fb_width = game::engine::rtarg.get_fb_width();
        int fb_height = game::engine::rtarg.get_fb_height();

        int tex_width = game::engine::rtarg.get_fb_tex_width();
        int tex_height = game::engine::rtarg.get_fb_tex_height();

        new_tex_width = game::utils::next_pow2(fb_width);
        new_tex_height = game::utils::next_pow2(fb_height);

        if (tex_width != new_tex_width || tex_height != new_tex_height){
            game::engine::rtarg.set_fb_tex_width(new_tex_width);
            game::engine::rtarg.set_fb_tex_height(new_tex_height);

            game::engine::rtarg.resize_render_target(game::engine::rtarg.get_fb_tex_width(),
                game::engine::rtarg.get_fb_tex_height());
        }

        game::engine::update_rendering_quad_tc();
    }
}

void CreditsScreen::update()
{
	game::engine::audio_manager->play_stream_playlist(&playlist, 1.0);

	if (skip || strings.empty()) {
        game::engine::nuc_manager->dump_emitters();
        game::engine::audio_manager->stop_streams();
        game::engine::audio_manager->stop_sources();

        for (int i = 0; i < 2; i++){
            game::engine::scrn_manager->pop_screen();
        }

        game::engine::scrn_manager->init_active_scrn();

        skip = false;
    }
	else {
		if ((get_msec() - start_time > 15990)) {
			strings.pop();
			if (!strings.empty())
				credits_txt->set_text(strings.top());
			start_time = get_msec();
		}
	}
}

void CreditsScreen::add_scene(Scene *sc)
{
	scenes.push_back(sc);
}

void CreditsScreen::map_scene_by_name(std::string name, Scene *sc)
{
	credits_scrn_scenes_by_name[name] = sc;
}

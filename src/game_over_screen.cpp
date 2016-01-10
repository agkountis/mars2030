#include "game_over_screen.h"
#include "game.h"

GameOverScreen::GameOverScreen()
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
}

GameOverScreen::~GameOverScreen()
{
    if(!scenes.empty()) {
        std::list<Scene*>::iterator it = scenes.begin();

        while(it != scenes.end()) {
            delete (*it);
            it = scenes.erase(it);
        }
    }

    if(lt)
        delete lt;
}

/*Private Functions*/
void GameOverScreen::render_default()
{
	lt->set_light();
	set_unistate("st_light_intensity", (float)1.0);

	game::engine::rtarg.bind(RT_COLOR_AND_DEPTH);
	glClearColor(0.3, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	game::engine::current_sdr = game::engine::cubemap_sdr;
	glDisable(GL_DEPTH_TEST);
	sky.render(RENDER_OBJ);
	glEnable(GL_DEPTH_TEST);

	game::engine::current_sdr = game::engine::main_no_shad_sdr;

	view.reset_identity();
	view.rotate(Vector3(0, 1, 0), DEG_TO_RAD(-90));
	view.translate(Vector3(-100, 0, 0));
	set_view_matrix(view);

	std::list<Scene*>::iterator it = scenes.begin();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	game_over_scrn_scenes_by_name["mars"]->render(RENDER_OBJ, get_msec());

	if (draw_ship)
		game_over_scrn_scenes_by_name["base_ship"]->render(RENDER_OBJ, get_msec());
	glDisable(GL_BLEND);

	game::engine::nuc_manager->update(dt, get_msec());
	game::engine::nuc_manager->render(RENDER_PSYS, get_msec());

	game_over_txt.set_position(Vector3(99.7, 0, 0.223));
	game_over_txt.set_rotation(Vector3(0, 1, 0), DEG_TO_RAD(90));
	game_over_txt.set_scaling(Vector3(0.0003, 0.0003, 0.0003));
	game_over_txt.render(get_msec());

	game::engine::rtarg.unbind();
    game::utils::display_to_rendering_quad();
}

void GameOverScreen::render_ovr()
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

			/* --view/camera tranformation --
			* We need to construct a view matrix by combining all the information provided by
			* the oculus SDK, about the position and orientation of the user's head in the world.
			*/
			view.reset_identity();

			view.rotate(Vector3(0, 1, 0), DEG_TO_RAD(-90));
			view.translate(Vector3(-100, 0, 0));

			//Send the View matrix to the shader.
			set_view_matrix(xform_data.view * view);

			/*Render calls*/
			game::engine::current_sdr = game::engine::cubemap_sdr;
			glDisable(GL_DEPTH_TEST);
			sky.render(RENDER_OBJ);
			glEnable(GL_DEPTH_TEST);

			game::engine::current_sdr = game::engine::main_no_shad_sdr;

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			Scene *mars = game_over_scrn_scenes_by_name["mars"];

			glDisable(GL_DEPTH_TEST);
			mars->render(RENDER_OBJ, get_msec());
			glEnable(GL_DEPTH_TEST);

			if (draw_ship)
			game_over_scrn_scenes_by_name["base_ship"]->render(RENDER_OBJ, get_msec());

			glDisable(GL_BLEND);

			game::engine::nuc_manager->render(RENDER_PSYS, get_msec());

			game_over_txt.set_position(Vector3(99.4, 0, 0.223));
			game_over_txt.set_rotation(Vector3(0, 1, 0), DEG_TO_RAD(90));
			game_over_txt.set_scaling(Vector3(0.0003, 0.0003, 0.0003));
			game_over_txt.render(get_msec());
			/*-----------------------------------------------------------------------------------------*/
		}
	}

	is_visible = game::engine::ovr_manager.submit_ovr_frame();

	game::engine::ovr_manager.draw_ovr_mirror_texture(game::options::win_width, game::options::win_height);
}
/*--------------------------------------------------------------------*/

void GameOverScreen::add_scene(Scene *scene)
{
	scenes.push_back(scene);
}

bool GameOverScreen::init()
{
    BaseScreen::init();

	this->stop();
	this->reset();
	this->start();

	std::list<Scene*>::iterator it = scenes.begin();
	while (it != scenes.end()){
		Object *root = (*it)->get_object(0);
		game::utils::create_scene_psys_emitters(root);
		it++;
	}

	Scene *sc = game_over_scrn_scenes_by_name["mars"];
	sc->set_anim_speed(1, 0.3);
	Object *atmosphere = sc->get_object("ATMOSPHERE");
	atmosphere->set_specific_shader(game::engine::atmosphere_sdr);
	atmosphere->get_material()->alpha = 0.8;

	expl_done = false;
	draw_ship = true;

	//MAKE THE TEXT RENDER CORRECTLY
	game_over_txt.set_font(game::assets::font);
	game_over_txt.set_shader_program(game::engine::font_sdr);
	game_over_txt.set_shader_attr_indices(0, 3);
	game_over_txt.set_font_size(200);
	game_over_txt.set_font_color(Vector4(1, 0, 0, 1));
	game_over_txt.init();

	
	game_over_txt.set_text("GAME OVER");

	return true;
}

void GameOverScreen::render()
{
	if (game::options::ovr_enabled){
		render_ovr();
	}
	else{
		render_default();
	}
}

void GameOverScreen::on_mouse_down(int btn, int x, int y)
{
	
}

void GameOverScreen::on_mouse_up(int btn, int x, int y)
{

}

void GameOverScreen::on_double_click(int btn, int x, int y)
{

}

void GameOverScreen::on_move(int x, int y)
{

}

void GameOverScreen::on_drag(int x, int y)
{

}

void GameOverScreen::on_key_press(unsigned char key, int x, int y)
{

}

void GameOverScreen::on_key_release(unsigned char key, int x, int y)
{

}

void GameOverScreen::reshape(int x, int y)
{
	if (!game::options::ovr_enabled){
		int new_tex_width, new_tex_height;

		game::engine::rtarg.set_fb_width(x * game::options::ss_factor);
		game::engine::rtarg.set_fb_height(y * game::options::ss_factor);

		glViewport(0, 0, game::engine::rtarg.get_fb_width(), game::engine::rtarg.get_fb_height());
		set_unistate("st_screen_size", Vector2(game::engine::rtarg.get_fb_tex_width(), game::engine::rtarg.get_fb_height()));
		float aspect = (float)game::engine::rtarg.get_fb_width() / (float)game::engine::rtarg.get_fb_height();

		projection_mat.set_perspective(DEG_TO_RAD(65.0), aspect, 0.1, 40000);
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
			std::cout << "Resized render target!" << std::endl;
		}

		game::engine::update_rendering_quad_tc();
	}
}

void GameOverScreen::update()
{
	if (get_msec() > 1700 && !expl_done){
		draw_ship = false;
		game::engine::nuc_manager->dump_emitters();
		expl_emitter = new NucEmitter;
		NucEmitterConfig conf;
		conf.size = 50.0;
		conf.spawn_radius = 0.2;
		conf.max_particles = 40.0;
		conf.spawn_rate = 300.0;
		conf.tex_path = "data/texture/particles/expl01.png";
		conf.start_color = Vector4(1, 0.3, 0, 1);
		conf.end_color = Vector4(1, 0, 0, 0);
		conf.lifespan = 10;
		conf.emission_duration = 10000;

		expl_emitter->set_emitter_config(conf);
		expl_emitter->set_marked_for_death(true);
		expl_emitter->set_renderer(game::engine::ps_r);
		expl_emitter->set_physics_simulator(game::engine::physics_sims[game::engine::physics_sim_idx_by_name["huge_explosion"]]);
		expl_emitter->init();
		expl_emitter->set_activation_time(get_msec());
		game::engine::nuc_manager->add_emitter(expl_emitter);
		expl_done = true;
	}

	if (get_msec() > 11000){
		game::engine::nuc_manager->dump_emitters();
		game::engine::audio_manager->stop_streams();
		game::engine::audio_manager->stop_sources();

		for (int i = 0; i < 2; i++){
			game::engine::scrn_manager->pop_screen();
		}

		game::engine::scrn_manager->init_active_scrn();
	}
}

void GameOverScreen::map_scene_by_name(std::string name, Scene *sc)
{
	game_over_scrn_scenes_by_name[name] = sc;
}

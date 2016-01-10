#include "starting_screen.h"
#include "game.h"

StartingScreen::StartingScreen()
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

StartingScreen::~StartingScreen()
{
	if (!scenes.empty()){
		std::list<Scene*>::iterator it = scenes.begin();

		while (it != scenes.end()){
			delete (*it);
			it = scenes.erase(it);
		}
	}

	if (lt)
		delete lt;
}

/*Private functions*/

void StartingScreen::render_billboard_scrn_tex()
{
	/*Store the matrices temporarily*/
	Matrix4x4 prev_proj = get_projection_matrix();
	Matrix4x4 prev_view = get_view_matrix();
	int prev_viewport[4];
	glGetIntegerv(GL_VIEWPORT, prev_viewport);
	/*--------------------------------------------*/

	glViewport(0, 0, billboard_scrn_rtarg.get_fb_width(), billboard_scrn_rtarg.get_fb_height());

	billboard_scrn_rtarg.bind(RT_COLOR_AND_DEPTH);
    glClearColor(0, 0, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	set_view_matrix(Matrix4x4());
	set_projection_matrix(Matrix4x4());
	set_world_matrix(Matrix4x4());
	game::engine::holomap_4dnoise_sdr->bind();
	glBindTexture(GL_TEXTURE_2D, billboard_tex->get_id());
	game::engine::rendering_quad->render();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	title.set_position(Vector3(-0.023, -0.1, 0.0));
	title.set_rotation(Vector3(1, 0, 0), DEG_TO_RAD(180));
    title.set_scaling(Vector3(0.0026, 0.0026, 1.0));
	title.set_text("MARS");
	title.render(get_msec());

	title.set_position(Vector3(-0.023, 0.27, 0.0));
	title.set_text("2030");
	title.render(get_msec());
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	billboard_scrn_rtarg.unbind();

	set_view_matrix(prev_view);
	set_projection_matrix(prev_proj);
	glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
}

void StartingScreen::render_default()
{
	lt->set_light();
	set_unistate("st_light_intensity", (float)1.0);

	render_billboard_scrn_tex();

	game::engine::rtarg.bind(RT_COLOR_AND_DEPTH);
    glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	game::engine::current_sdr = game::engine::cubemap_sdr;
	glDisable(GL_DEPTH_TEST);
	sky.render(RENDER_OBJ);
	glEnable(GL_DEPTH_TEST);

	game::engine::current_sdr = game::engine::main_no_shad_sdr;
	
	view.reset_identity();
	view.rotate(Vector3(0, 1, 0), DEG_TO_RAD(-55));
	set_view_matrix(view);


    Scene *mars = starting_scrn_scenes_by_name["mars"];
    Scene *space_billboard = starting_scrn_scenes_by_name["space_billboard"];

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    mars->render(RENDER_OBJ, get_msec());
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);
    space_billboard->render(RENDER_OBJ, get_msec());
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);


	game::engine::nuc_manager->update(dt, get_msec());
	game::engine::nuc_manager->render(RENDER_PSYS, get_msec());

	start_message.render(get_msec());

	game::engine::rtarg.unbind();

    game::utils::display_to_rendering_quad();
}

void StartingScreen::render_ovr()
{
	static bool is_visible = true;

	if (!game::engine::ovr_initialized){
		std::cout << "Ovr HMD not initialized!" << std::endl;
		return;
	}

	lt->set_light();

	set_unistate("st_light_intensity", (float)1.0);
	
	render_billboard_scrn_tex();

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
		
			view.rotate(Vector3(0, 1, 0), DEG_TO_RAD(-55));
			
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
			Scene *mars = starting_scrn_scenes_by_name["mars"];
			Scene *space_billboard = starting_scrn_scenes_by_name["space_billboard"];

			glDisable(GL_DEPTH_TEST);
			mars->render(RENDER_OBJ, get_msec());
			glEnable(GL_DEPTH_TEST);

			space_billboard->render(RENDER_OBJ, get_msec());

			glDisable(GL_BLEND);

			game::engine::nuc_manager->render(RENDER_PSYS, get_msec());

			start_message.render(get_msec());
			/*-----------------------------------------------------------------------------------------*/
		}
	}
	
	is_visible = game::engine::ovr_manager.submit_ovr_frame();

	game::engine::ovr_manager.draw_ovr_mirror_texture(game::options::win_width, game::options::win_height);
}
/*------------------*/

void StartingScreen::add_scene(Scene *scene)
{
	scenes.push_back(scene);
}

bool StartingScreen::init()
{
    BaseScreen::init();

	this->stop();
	this->reset();
	this->start();

	Scene *sc = NULL;
	Scene *sc2 = NULL;

    game::engine::warp_mouse = true;

	sc = starting_scrn_scenes_by_name["mars"];
	if (sc){
		sc->set_anim_speed(1, 0.3);
		Object* atmosphere = NULL;
		atmosphere = sc->get_object("ATMOSPHERE");
		if (atmosphere){
			atmosphere->set_specific_shader(game::engine::atmosphere_sdr);
			atmosphere->get_material()->alpha = 0.8;
		}
	}

	sc2 = starting_scrn_scenes_by_name["space_billboard"];
	if (sc2)
		sc2->set_anim_speed(1, 0.1);

	billboard_scrn_rtarg.set_fb_width(1024);
	billboard_scrn_rtarg.set_fb_height(1024);
	billboard_scrn_rtarg.create(RT_COLOR_AND_DEPTH_MASK);

	billboard_tex = game::engine::texset->get("data/textures/spaceships/screen_background.jpg");

	start_message.set_font(game::assets::font);
	start_message.set_shader_program(game::engine::flashing_text_sdr);
	start_message.set_shader_attr_indices(0, 3);
	start_message.set_font_size(200);
	start_message.set_font_color(Vector4(1, 0, 0, 1));
	start_message.init();

	start_message.set_position(Vector3(-0.45, -0.2, -0.3));
	start_message.set_rotation(Vector3(0, 1, 0), DEG_TO_RAD(55));
    start_message.set_scaling(Vector3(0.0002, 0.0002, 1.0));
	start_message.set_text("Click to start!");

	title.set_font(game::assets::font);
	title.set_shader_program(game::engine::font_sdr);
	title.set_shader_attr_indices(0, 3);
	title.set_font_size(200);
	title.set_font_color(Vector4(1, 0, 0, 1));
	title.init();
	title.set_text_alignment(TXT_CENTER_ALIGNMENT);


	Object *billboard_scrn = NULL;
	billboard_scrn = sc2->get_object("SCREEN");

	if (billboard_scrn){
		goatgfx::Texture *tex = new goatgfx::Texture2D;
        game::engine::texset->add("billboard_scrn_tex", tex);
		Material *mat = billboard_scrn->get_material();
		tex->set_id(billboard_scrn_rtarg.get_color_attachment());
		mat->textures[TEX_DIFFUSE] = tex;

		billboard_scrn->set_specific_shader(game::engine::no_lighting_sdr);
	}

	std::list<Scene*>::iterator it = scenes.begin();

	while (it != scenes.end()){
		Object *root = (*it)->get_object(0);
		game::utils::create_scene_psys_emitters(root);
		it++;
	}

	plist.files.clear();
	plist.files.push_back("data/audio/backyard_ghosts.ogg");
	plist.it = plist.files.begin();
	plist.loop = true;

	game::engine::audio_manager->play_sample(game::utils::get_audio_sample_by_name("rocket"), 0.1, AUDIO_PLAYMODE_LOOP);
	
	return true;
}

void StartingScreen::render()
{
	if (game::options::ovr_enabled){
		render_ovr();
	}
	else{
		render_default();
	}
}

void StartingScreen::on_mouse_down(int btn, int x, int y)
{
	game::engine::nuc_manager->dump_emitters();
	game::engine::audio_manager->stop_streams();
	game::engine::audio_manager->stop_sources();
	game::engine::scrn_manager->push_screen(game::engine::game_scrn);
	game::engine::scrn_manager->init_active_scrn();
}

void StartingScreen::on_mouse_up(int btn, int x, int y)
{

}

void StartingScreen::on_double_click(int btn, int x, int y)
{

}

void StartingScreen::on_move(int x, int y)
{

}

void StartingScreen::on_drag(int x, int y)
{

}

void StartingScreen::on_key_press(unsigned char key, int x, int y)
{

}

void StartingScreen::on_key_release(unsigned char key, int x, int y)
{

}

void StartingScreen::reshape(int x, int y)
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

void StartingScreen::update()
{
	set_unistate("st_time_msec", (float)get_msec());
	game::engine::audio_manager->play_stream_playlist(&plist, 1.0);
}

void StartingScreen::map_scene_by_name(std::string name, Scene *sc)
{
	starting_scrn_scenes_by_name[name] = sc;
}

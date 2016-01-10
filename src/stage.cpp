#include "stage.h"
#include "game.h"
#include "texture.h"

Stage::Stage()
{
    lt = new Light;
    lt->set_position(Vector3(70000, 70000, -70000));
    lt->set_att_constant(1);
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

	cockpit_sc = NULL;
    cockpit_ctrl = NULL;

	playlist.files.push_back("data/audio/breach2.ogg");
	playlist.files.push_back("data/audio/backyard_ghosts.ogg");
	playlist.loop = true;

	health = 100;

	tutorial_started = false;
	skip_tutorial = false;
	tutorial_finished = false;
	tutorial_src = NULL;
    stage_over = false;

	kill_count = 0;
}

Stage::~Stage()
{
    if(!stage_scenes.empty()){

        std::list<Scene*>::iterator it = stage_scenes.begin();

        while(it != stage_scenes.end()){
            delete *it;
            it = stage_scenes.erase(it);
        }
    }

	if (lt)
		delete lt;

    if(cockpit_sc)
        delete cockpit_sc;
}

/*Private Functions---------------------------------------*/
void Stage::render_sky(long time)
{
    glDepthMask(GL_FALSE);
    game::engine::current_sdr = game::engine::cubemap_sdr;
    sky.render(RENDER_OBJ);
    game::engine::current_sdr =	game::engine::main_sdr;
    glDepthMask(GL_TRUE);
}

void Stage::render_scenes(long time)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /*if(!stage_scenes.empty()){
        std::list<Scene*>::iterator it = stage_scenes.begin();

        while(it != stage_scenes.end()){
            (*it)->render(RENDER_OBJ, time);
            it++;
        }
    }*/
	Scene *sc = scene_ptr_by_name["mars2.fbx"];
	
	glDisable(GL_DEPTH_TEST);
	sc->render(RENDER_OBJ, time);
	glEnable(GL_DEPTH_TEST);

	sc = scene_ptr_by_name["base_ship5.fbx"];
	sc->render(RENDER_OBJ, time);

	glDisable(GL_BLEND);
}

void Stage::render_enemies(long time)
{
    wave_group_manager.render(RENDER_OBJ, time);
}

void Stage::render_cockpit(long time)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	cockpit_sc->render(RENDER_OBJ, time);
	glDisable(GL_BLEND);
}

void Stage::render_holomap(long time)
{
	set_unistate("st_time_msec", (float)time);
	holomap.render(RENDER_OBJ, time);
}

void Stage::render_cockpit_screens(long time)
{
	/*Store the matrices temporarily*/
	Matrix4x4 prev_proj = get_projection_matrix();
	Matrix4x4 prev_view = get_view_matrix();
	int prev_viewport[4];
	glGetIntegerv(GL_VIEWPORT, prev_viewport);
	/*--------------------------------------------*/

	glDisable(GL_DEPTH_TEST);

	/*Left Screen texture rendering*/
	left_scrn_rtarg.bind(RT_COLOR_AND_DEPTH);
	glClearColor(1.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_projection_matrix(Matrix4x4());
	set_view_matrix(Matrix4x4());
	set_world_matrix(Matrix4x4());

	glViewport(0, 0, left_scrn_rtarg.get_fb_width(), left_scrn_rtarg.get_fb_height());

	game::engine::fb_color_sdr->bind();
	glBindTexture(GL_TEXTURE_2D, background);
	game::engine::rendering_quad->render();
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_CULL_FACE);
	text.set_position(Vector3(0, -0.4, 0));
	text.set_rotation(Vector3(1, 0, 0), DEG_TO_RAD(180));
	text.set_scaling(Vector3(0.00118, 0.00118, 0.00118));
	text.set_text("STRUCTURE");
	text.render(time);

	text.set_position(Vector3(0, 0.4, 0));
	text.set_scaling(Vector3(0.0036, 0.0036, 0.0036));
	char buff[16];
	sprintf(buff, "%d", health);
	text.set_text((std::string)buff);
	text.render(time);
	glEnable(GL_CULL_FACE);
	left_scrn_rtarg.unbind();
	/*------------------------------------------------------------------------------------------------*/

	/*Right Screen texture rendering*/
	right_scrn_rtarg.bind(RT_COLOR_AND_DEPTH);
	glClearColor(0.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_projection_matrix(Matrix4x4());
	set_world_matrix(Matrix4x4());

	game::engine::fb_color_sdr->bind();
	glBindTexture(GL_TEXTURE_2D, background);
	game::engine::rendering_quad->render();
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_CULL_FACE);
	text.set_position(Vector3(0, -0.4, 0));
	text.set_rotation(Vector3(1, 0, 0), DEG_TO_RAD(180));
    text.set_scaling(Vector3(0.00118, 0.00118, 1.0));
	text.set_text("HEAT");
	text.render(time);

	text.set_position(Vector3(0, 0.4, 0));
    text.set_scaling(Vector3(0.0036, 0.0036, 1.0));
	sprintf(buff, "%d", weapon_system.get_heat());
	text.set_text((std::string)buff);
	text.render(time);
	glEnable(GL_CULL_FACE);

	right_scrn_rtarg.unbind();
	/*--------------------------------------------------------------------------------------------------*/

	if (game::options::ovr_enabled) {
		/*Far left Screen texture rendering*/
		far_left_scrn_rtarg.bind(RT_COLOR_AND_DEPTH);
		glClearColor(1.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		set_projection_matrix(Matrix4x4());
		set_world_matrix(Matrix4x4());

		game::engine::fb_color_sdr->bind();
		glBindTexture(GL_TEXTURE_2D, background);
		game::engine::rendering_quad->render();
		glBindTexture(GL_TEXTURE_2D, 0);

		glDisable(GL_CULL_FACE);
		text.set_position(Vector3(0, -0.4, 0));
		text.set_rotation(Vector3(1, 0, 0), DEG_TO_RAD(180));
		text.set_scaling(Vector3(0.00118, 0.00118, 1.0));
		text.set_text("COOLANT");
		text.render(time);

		std::string s;
		int charges = weapon_system.get_coolant_charges();
		for (unsigned int i = 0; i < charges; i++){
			s.append("+");
		}
		text.set_position(Vector3(0, 0.4, 0));
		text.set_scaling(Vector3(0.0036, 0.0036, 1.0));
		text.set_text(s);
		text.render(time);

		far_left_scrn_rtarg.unbind();
		glEnable(GL_CULL_FACE);
		/*---------------------------------------------------------------------------------------------------*/

		/*Far right Screen texture rendering*/
		far_right_scrn_rtarg.bind(RT_COLOR_AND_DEPTH);
		glClearColor(1.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		set_projection_matrix(Matrix4x4());
		set_world_matrix(Matrix4x4());

		game::engine::fb_color_sdr->bind();
		glBindTexture(GL_TEXTURE_2D, background);
		game::engine::rendering_quad->render();
		glBindTexture(GL_TEXTURE_2D, 0);

		glDisable(GL_CULL_FACE);
		text.set_position(Vector3(0, -0.4, 0));
		text.set_rotation(Vector3(1, 0, 0), DEG_TO_RAD(180));
		text.set_scaling(Vector3(0.00118, 0.00118, 1.0));
		text.set_text("KILL COUNT");
		text.render(time);

		sprintf(buff, "%d", kill_count);
		text.set_text((std::string)buff);

		text.set_position(Vector3(0, 0.4, 0));
		text.set_scaling(Vector3(0.0036, 0.0036, 1.0));
		text.render(time);

		far_left_scrn_rtarg.unbind();
		glEnable(GL_CULL_FACE);
		/*---------------------------------------------------------------------------------------------------*/
	}
	
	glEnable(GL_DEPTH_TEST);

	/*Revert to the original matrices*/
	set_projection_matrix(prev_proj);
	set_view_matrix(prev_view);
	glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
	/*----------------------------------------------------------------------------------*/
}

void Stage::render_hud(long time)
{
	hud_text.set_position(Vector3(-0.26, 0.22, -0.4));
    hud_text.set_scaling(Vector3(0.00008, 0.00008, 1.0));
	char buff[24];
	sprintf(buff, "STRUCTURE\n     %d", health);
	hud_text.set_text((std::string)buff);
	hud_text.render(time);

	hud_text.set_position(Vector3(0.18, 0.22, -0.4));
    hud_text.set_scaling(Vector3(0.00008, 0.00008, 1.0));
	sprintf(buff, "HEAT\n   %d", weapon_system.get_heat());
	hud_text.set_text((std::string)buff);
	hud_text.render(time);

	hud_text.set_position(Vector3(-0.26, -0.22, -0.4));
    hud_text.set_scaling(Vector3(0.00008, 0.00008, 1.0));
	std::string s;
	s = "    ";
	int charges = weapon_system.get_coolant_charges();
	for (unsigned int i = 0; i < charges; i++){
		s.append("+ ");
	}
	s.append("\nCOOLANT");
	hud_text.set_text(s);
	hud_text.render(time);

	hud_text.set_position(Vector3(0.14, -0.22, -0.4));
    hud_text.set_scaling(Vector3(0.00008, 0.00008, 1.0));
	sprintf(buff, "       %d\nKILL COUNT", kill_count);
	hud_text.set_text((std::string)buff);
	hud_text.render(time);
}

void Stage::render_hud_ovr(long time)
{
	//-0.13 0.09 -0.2
	//0.00006
	hud_text.set_position(Vector3(-0.23, 0.22, -0.4));
    hud_text.set_scaling(Vector3(0.00008, 0.00008, 1.0));
	char buff[24];
	sprintf(buff, "STRUCTURE\n     %d", health);
	hud_text.set_text((std::string)buff);
	hud_text.render(time);

	//0.08 0.09 -0.2
	//0.00006
	hud_text.set_position(Vector3(0.18, 0.22, -0.4));
    hud_text.set_scaling(Vector3(0.00008, 0.00008, 1.0));
	sprintf(buff, "HEAT\n   %d", weapon_system.get_heat());
	hud_text.set_text((std::string)buff);
	hud_text.render(time);

	//-0.04 -0.09 -0.2
	//0.00006
	hud_text.set_position(Vector3(-0.23, -0.22, -0.4));
    hud_text.set_scaling(Vector3(0.00008, 0.00008, 1.0));
	std::string s;
	int charges = weapon_system.get_coolant_charges();
	for (unsigned int i = 0; i < charges; i++){
		s.append("+");
	}
	s.append("\nCOOLANT");
	hud_text.set_text(s);
	hud_text.render(time);

	hud_text.set_position(Vector3(0.2, -0.22, -0.4));
    hud_text.set_scaling(Vector3(0.00008, 0.00008, 1.0));
	sprintf(buff, "SCORE");
	hud_text.set_text((std::string)buff);
	hud_text.render(time);
}

void Stage::render_crosshair(long time)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	crosshair.render(RENDER_OBJ, time);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

bool Stage::load_scene(const char *path)
{
    Scene *sc = new Scene;
    if(!sc->load(path)){
        return false;
    }

    if(!sc->load_animation(path, "scene_anim", true)){
        std::cout<<"Failed to load the animation of "<<path<<std::endl;
    }else{
        sc->start_animation(1, 0);
    }

	std::string p = (std::string)path;
	unsigned int idx1 = p.find_last_of("/");
	p = p.substr(idx1 + 1, p.size());
	scene_ptr_by_name[p] = sc;

    stage_scenes.push_back(sc);
    return true;
}

bool Stage::load_waves(const NCF &ncf)
{
	NCF *node = ncf.get_group_by_name("stage_waves");
	if (!node){
		std::cerr << "Null NCF node! Cannot load the wave data!" << std::endl;
	}

	for (unsigned int i = 0; i < node->count_groups(); i++){
		WaveGroup *wv_grp = new WaveGroup;
		NCF *sub_grp1 = node->get_group_by_index(i);

		std::string audio_sample_name = sub_grp1->get_property_by_name("audio_sample");
		wv_grp->assign_audio_sample(audio_sample_name.c_str());

		for (unsigned int j = 0; j < sub_grp1->count_groups(); j++){
			Wave *wv = new Wave;
			NCF *sub_grp2 = sub_grp1->get_group_by_index(j);
			wv->set_init_delay(atoi(sub_grp2->get_property_by_name("init_delay")));
			

			for (unsigned int k = 0; k < sub_grp2->count_groups(); k++){
				EnemyGenerator *en_gen = new EnemyGenerator;
				NCF *sub_grp3 = sub_grp2->get_group_by_index(k);

				en_gen->set_type((std::string)sub_grp3->get_property_by_name("type"));
				en_gen->set_amount(atoi(sub_grp3->get_property_by_name("amount")));

				SphericalSpawnData data;
				data.radius = atof(sub_grp3->get_property_by_name("radius"));
				data.phi = atof(sub_grp3->get_property_by_name("phi"));
				data.theta = atof(sub_grp3->get_property_by_name("theta"));
				data.spawn_radius = atof(sub_grp3->get_property_by_name("spawn_radius"));

				en_gen->set_spherical_spawn_data(data);
				en_gen->set_generation_rate(atof(sub_grp3->get_property_by_name("spawn_rate")));
				en_gen->assign_wave(wv);
				wv->add_generator(en_gen);
			}
			wv_grp->add_wave(wv);
		}
		wave_group_manager.add_wave_group(wv_grp);
	}

	return true;
}

void Stage::setup_psys_emitters()
{
    if(!stage_scenes.empty()){
        std::list<Scene*>::iterator it = stage_scenes.begin();

        while(it != stage_scenes.end()){
            for(unsigned int i = 0 ; i < (*it)->get_object_count() ; i++){
                Object *root = (*it)->get_object(i);
                game::utils::create_scene_psys_emitters(root);
            }
            it++;
        }
    }
}
/*---------------------------------------------------------*/

void Stage::set_health(int hp)
{
	health = hp;
}

void Stage::set_shooting_state(bool state)
{
	weapon_system.set_shooting_state(state);
}

int Stage::get_health() const
{
	return health;
}

Matrix4x4* Stage::get_camera_matrix()
{
    return &camera;
}

XFormNode* Stage::get_gun_ctrl_node_ptr()
{
	return weapon_system.get_gun_ctrl_node_ptr();
}

XFormNode* Stage::get_cockpit_ctrl_node_ptr()
{
	return cockpit_ctrl;
}

Scene* Stage::get_cockpit_scene_ptr()
{
	return cockpit_sc;
}

Holomap* Stage::get_holomap_ptr()
{
	return &holomap;
}

XFormNode* Stage::get_hud_ctrl_node_ptr()
{
	return &hud_ctrl_node;
}

WeaponSystem* Stage::get_weapon_system_ptr()
{
	return &weapon_system;
}

Object* Stage::get_crosshair()
{
	return &crosshair;
}

WaveGroupManager* Stage::get_wave_group_manager_ptr()
{
	return &wave_group_manager;
}

Light* Stage::get_light()
{
	return lt;
}

void Stage::set_skip_tutorial_state(bool state)
{
	skip_tutorial = state;
}

bool Stage::has_tutorial_started() const
{
	return tutorial_started;
}

bool Stage::load(const char *fname)
{
    NCF ncf;
    std::string path;
    std::list<Scene*>::iterator it;
    Scene *sc = NULL;

    ncf.set_source(fname);

    if(ncf.parse() != 0){
        std::cout<<"Could not parse "<<fname<<". Aborting!"<<std::endl;
        return false;
    }

    NCF *node = ncf.get_group_by_name("stage_scenes");
    if(!node){
        std::cout<<"Could not parse the stage scenes. Aborting!"<<std::endl;
        return false;
    }

    for(unsigned int i = 0 ; i < node->count_properties() ; i++){
        std::string path = node->get_property_by_index(i);
        if(!load_scene(path.c_str())){
            std::cout<<"Failed to load "<<path<<". Aborting!"<<std::endl;
            return false;
        }
    }

    node = ncf.get_group_by_name("stage_guns");
    if(!node){
        std::cout<<"Could not parse the stage guns. Aborting!"<<std::endl;
        return false;
    }

	for (unsigned int i = 0; i < NUM_GUNS; i++){
		path = node->get_property_by_index(i);
		if (!weapon_system.load_gun((GunPosition)i, path.c_str())){
			return false;
		}
	}
	weapon_system.load_gun_anim(GUN_LEFT, "data/models/left_gun_shoot2.fbx", "shoot", false);
	weapon_system.load_gun_anim(GUN_RIGHT, "data/models/right_gun_shoot2.fbx", "shoot", false);

    node = ncf.get_group_by_name("stage_cockpit");
    if(!node){
        std::cout<<"Could not parse the stage cockpit. Aborting!"<<std::endl;
        return false;
    }
    path = node->get_property_by_index(0);
	cockpit_sc = new Scene;
	cockpit_sc->load(path.c_str());
    if(!cockpit_sc){
        std::cout<<"Failed to load "<<path<<". Aborting!"<<std::endl;
        return false;
    }
    

	/*Wave loading-----------------------------------------------------------------------------------------*/
	if (!load_waves(ncf))
		return false;
	/*------------------------------------------------------------------------------------------------------*/

    return true;
}

bool Stage::init()
{
	left_scrn_rtarg.set_fb_width(512);
	left_scrn_rtarg.set_fb_height(512);
	left_scrn_rtarg.create(RT_COLOR_AND_DEPTH_MASK);

	right_scrn_rtarg.set_fb_width(512);
	right_scrn_rtarg.set_fb_height(512);
	right_scrn_rtarg.create(RT_COLOR_AND_DEPTH_MASK);

	far_right_scrn_rtarg.set_fb_width(512);
	far_right_scrn_rtarg.set_fb_height(512);
	far_right_scrn_rtarg.create(RT_COLOR_AND_DEPTH_MASK);

	far_left_scrn_rtarg.set_fb_width(512);
	far_left_scrn_rtarg.set_fb_height(512);
	far_left_scrn_rtarg.create(RT_COLOR_AND_DEPTH_MASK);

	background = game::engine::texset->get("/data/textures/spaceships/screen_background.jpg")->get_id();
	
	text.set_font(game::assets::font);
	text.set_shader_program(game::engine::font_sdr);
	text.set_shader_attr_indices(0, 3);
	text.set_font_size(200);
	text.set_text_alignment(TXT_CENTER_ALIGNMENT);
	text.set_font_color(Vector4(1, 0, 0, 1));
	text.init();

	hud_text.set_font(game::assets::font);
	hud_text.set_shader_program(game::engine::font_sdr);
	hud_text.set_shader_attr_indices(0, 3);
	hud_text.set_font_size(200);
	hud_text.set_font_color(Vector4(1, 0, 0, 0.8));
	//hud_text.set_text_alignment(TXT_CENTER_ALIGNMENT);
	hud_text.init();

	hud_ctrl_node.add_child((XFormNode*)&hud_text);

	holomap.set_max_detect_dist(500.0);
	holomap.set_max_gun_range(300.0);
	holomap.init();

	crosshair.set_mesh(game::engine::rendering_quad);
	crosshair.set_mesh_ownership(false);
	crosshair.set_position(Vector3(0, 0, -5000));
	crosshair.set_scaling(Vector3(600, 600, 600));
	crosshair.set_specific_shader(game::engine::crosshair_sdr);

	Material *mat = crosshair.get_material();
	mat->textures[TEX_DIFFUSE] = game::engine::texset->get("data/textures/spaceships/my_crosshair2.png");
	mat->diffuse = Vector4(1.0, 0.0, 0.0, 1.0);

	/*initialisations-----------------------------------------------------------------------------------------------*/
	cockpit_ctrl = (XFormNode*)cockpit_sc->get_object("ROOT_$AssimpFbx$_PreRotation");

	Object *glass = cockpit_sc->get_object("GLASS");
	if (glass){
		glass->set_specific_shader(game::engine::fresnel_sdr);
		glass->set_shadow_casting(false);

		Material *mat = glass->get_material();
		mat->textures[TEX_DIFFUSE] = game::engine::texset->get("data/textures/cubemaps/cockpit_cubemap.png");
	}

	Object *map_dummy = cockpit_sc->get_object("MAP_POS_DUMMY");
	if (map_dummy){
		holomap.set_attachment_dummy((XFormNode*)map_dummy);
	}

	Object *left_scrn = cockpit_sc->get_object("SCREEN_CL");
	if (left_scrn){
		left_scrn->set_specific_shader(game::engine::no_lighting_sdr);
		Material *mat = left_scrn->get_material();
		goatgfx::Texture *tex = new goatgfx::Texture2D;
		tex->set_id(left_scrn_rtarg.get_color_attachment());
		mat->textures[TEX_DIFFUSE] = tex;
	}

	Object *right_scrn = cockpit_sc->get_object("SCREEN_CR");
	if (right_scrn){
		right_scrn->set_specific_shader(game::engine::no_lighting_sdr);
		Material *mat = right_scrn->get_material();
		goatgfx::Texture *tex = new goatgfx::Texture2D;
		tex->set_id(right_scrn_rtarg.get_color_attachment());
		mat->textures[TEX_DIFFUSE] = tex;
	}

	Object *far_left_scrn = cockpit_sc->get_object("SCREEN_L");
	if (far_left_scrn) {
		far_left_scrn->set_specific_shader(game::engine::no_lighting_sdr);
		Material *mat = far_left_scrn->get_material();
		goatgfx::Texture *tex = new goatgfx::Texture2D;
		tex->set_id(far_left_scrn_rtarg.get_color_attachment());
		mat->textures[TEX_DIFFUSE] = tex;
	}

	Object *far_right_scrn = cockpit_sc->get_object("SCREEN_R");
	if (far_right_scrn) {
		far_right_scrn->set_specific_shader(game::engine::no_lighting_sdr);
		Material *mat = far_right_scrn->get_material();
		goatgfx::Texture *tex = new goatgfx::Texture2D;
		tex->set_id(far_right_scrn_rtarg.get_color_attachment());
		mat->textures[TEX_DIFFUSE] = tex;
	}

	Scene *sc = NULL;
	sc = scene_ptr_by_name["mars2.fbx"];
	if (sc){
		sc->set_curr_anim_speed(0.3);
		Object *obj = sc->get_object("MARS");
		if (obj)
			obj->set_specific_shader(game::engine::main_no_shad_sdr);

		obj = sc->get_object("ATMOSPHERE");
		if (obj){
			obj->set_specific_shader(game::engine::atmosphere_sdr);
			obj->get_material()->alpha = 0.8;
		}
	}
	/*------------------------------------------------------------------------------*/

	weapon_system.init();
	setup_psys_emitters();

	tutorial = game::assets::audio_samples[game::assets::audio_sample_idx_by_name["tutorial"]];

    return true;
}

void Stage::setup_lights()
{
	if (lt){
		set_unistate("st_light_intensity", (float)1.0);
		lt->set_light();
	}
}

void Stage::render(unsigned int render_mask, long time)
{
    if(render_mask &(1 << SKY)){
        render_sky(time);
    }

    if(render_mask &(1 << SCENES)){
		render_scenes(time);
    }

    if(render_mask &(1 << ENEMIES)){
		render_enemies(time);
    }

	if (render_mask &(1 << GUNS)){
		weapon_system.render_guns(RENDER_OBJ, time);
	}

	if (render_mask &(1 << PROJECTILES)){
		weapon_system.render_projectiles(RENDER_OBJ, time);
	}

	if (render_mask &(1 << COCKPIT)){
		render_cockpit(time);
	}

	if (render_mask &(1 << HOLOMAP)){
		render_holomap(time);
	}

	if (render_mask &(1 << COCKPIT_SCRN_TEXTURES)){
		render_cockpit_screens(time);
	}

	if (render_mask &(1 << RENDER_HUD)){
		render_hud(time);
	}

	if (render_mask &(1 << RENDER_HUD_OVR)){
		render_hud_ovr(time);
	}

	if (render_mask &(1 << RENDER_CROSSHAIR)){
		render_crosshair(time);
	}
}

void Stage::update(long time, float dt)
{
	Vector3 vec = Vector3(0, 0, -1);
	vec.transform((camera).get_rotation_quat());
	game::engine::audio_manager->set_listener_orientation(vec);

	if (!tutorial_started) {
		game::engine::audio_manager->play_sample(tutorial, 1.0, AUDIO_PLAYMODE_ONCE);
		tutorial_started = true;
		tutorial_src = game::engine::audio_manager->get_audio_source(tutorial);
	}
	else {
        if ((skip_tutorial || (tutorial_src && !tutorial_src->is_playing())) && !tutorial_finished) {
			tutorial_src->stop();
			tutorial_finished = true;
			skip_tutorial = false;
		}
	}

	if (tutorial_finished) {
		game::engine::audio_manager->play_stream_playlist(&playlist, 0.3);
		weapon_system.update(time, dt);
		wave_group_manager.update(dt, time);

        if(!wave_group_manager.has_wave_groups())
            stage_over = true;
	}

	holomap.update();
	wave_group_manager.remove_defeated_enemies();

	if (health <= 0){
		game::engine::nuc_manager->dump_emitters();
		game::engine::audio_manager->stop_streams();
		game::engine::audio_manager->stop_sources();

		game::engine::audio_manager->play_sample(game::assets::audio_samples[game::assets::audio_sample_idx_by_name["large_explosion"]], 284,
			AUDIO_PLAYMODE_ONCE);
		
		delete this;
		game::engine::active_stage = NULL;

		game::engine::scrn_manager->push_screen(game::engine::game_over_scrn);
		game::engine::scrn_manager->init_active_scrn();
		game::engine::scrn_manager->start_active_scrn();

		return;
	}

    if(stage_over) {

		int idx = game::assets::audio_sample_idx_by_name["victory"];
		AudioSample *sample = game::assets::audio_samples[idx];
		
		static bool is_playing = false;
		if (!is_playing) {
			game::engine::audio_manager->set_streams_volume(0.15);
			source.set_sample(sample);
			source.set_looping(false);
			source.set_volume(1.0);
			source.play();
			is_playing = true;
		}

		if (!source.is_playing()) {
			is_playing = false;
			game::engine::audio_manager->set_streams_volume(1.0);
			game::engine::nuc_manager->dump_emitters();
			game::engine::audio_manager->stop_streams();
			game::engine::audio_manager->stop_sources();

			delete this;
			game::engine::active_stage = NULL;

			game::engine::scrn_manager->push_screen(game::engine::credits_scrn);
			game::engine::scrn_manager->init_active_scrn();
			game::engine::scrn_manager->start_active_scrn();
		}
    }
}

void Stage::setup_psys_emitters(Scene *sc)
{
	for (unsigned int i = 0; i < sc->get_object_count(); i++){
		Object *root = sc->get_object(i);
		game::utils::create_scene_psys_emitters(root);
	}
}

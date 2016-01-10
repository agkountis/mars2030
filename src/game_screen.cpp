#include "game_screen.h"
#include "bitmap_text.h"
//DEBUG
static void render_axes();
//--

GameScreen::GameScreen()
{
	phi = 0;
	theta = 0;
}

GameScreen::~GameScreen()
{

}

bool GameScreen::init()
{
    BaseScreen::init();

	this->reset();
	game::engine::warp_mouse = true;

	if (!game::engine::active_stage){
		game::engine::active_stage = new Stage;
		if (!game::engine::active_stage->load("data/config/stage1.config")){
			std::cout << "Failed to load the stage! Aborting!" << std::endl;
		}
	}

	game::engine::active_stage->init();
	
	phi = 0;
	theta = 0;
	return true;
}

void GameScreen::render_default()
{
	game::engine::active_stage->setup_lights();

	camera = game::engine::active_stage->get_camera_matrix();
	camera->reset_identity();
	camera->rotate(Vector3(1, 0, 0), DEG_TO_RAD(phi));
	camera->rotate(Vector3(0, 1, 0), DEG_TO_RAD(theta));
	set_view_matrix(*camera);

	Matrix4x4 cockpit_loc_mat;

	XFormNode *gun_ctrl = game::engine::active_stage->get_gun_ctrl_node_ptr();
	XFormNode *cockpit_ctrl = game::engine::active_stage->get_cockpit_ctrl_node_ptr();
	Object *crosshair = game::engine::active_stage->get_crosshair();

	crosshair->set_matrix((*camera).inverse());

	cockpit_loc_mat.rotate(Vector3(0, 1, 0), DEG_TO_RAD(theta));
	cockpit_ctrl->set_matrix(cockpit_loc_mat.inverse());

	gun_ctrl->set_matrix((*camera).inverse());

	shadow_render_pass(false, get_msec());
	//draw_shad_tex();
	color_render_pass(get_msec());
    game::utils::display_to_rendering_quad();
}

void GameScreen::render_ovr()
{
	static bool is_visible = true;

	if (!game::engine::ovr_initialized){
		std::cout << "Ovr HMD not initialized!" << std::endl;
		return;
	}

	game::engine::active_stage->setup_lights();
	camera = game::engine::active_stage->get_camera_matrix();

	set_unistate("st_light_intensity", (float)1.0);

	

	Matrix4x4 gun_loc_mat;
	Matrix4x4 cockpit_loc_mat;
	Matrix4x4 crosshair_loc_mat;

	/*Gun and Cokcpit tranformations*/
	XFormNode *gun_ctrl = game::engine::active_stage->get_gun_ctrl_node_ptr();
	XFormNode *cockpit_ctrl = game::engine::active_stage->get_cockpit_ctrl_node_ptr();
	XFormNode *crosshair_crtl = (XFormNode*)game::engine::active_stage->get_crosshair();

	cockpit_loc_mat.rotate(Vector3(0, 1, 0), DEG_TO_RAD(theta));
	cockpit_ctrl->set_matrix(cockpit_loc_mat.inverse());

	gun_loc_mat.rotate(Vector3(1, 0, 0), DEG_TO_RAD(phi));
	gun_loc_mat.rotate(Vector3(0, 1, 0), DEG_TO_RAD(theta));
	gun_ctrl->set_matrix(gun_loc_mat.inverse());

	crosshair_loc_mat.rotate(Vector3(1, 0, 0), DEG_TO_RAD(phi));
	crosshair_loc_mat.rotate(Vector3(0, 1, 0), DEG_TO_RAD(theta));
	crosshair_crtl->set_matrix(crosshair_loc_mat.inverse());
	/*---------------------------------------------------------------------------*/

	//Shadow render pass.
	shadow_render_pass(true, get_msec());

	/*Render the textures of the cockpits front screens.*/
	game::engine::active_stage->render(STAGE_RENDER_COCKPIT_SCRN_TEXTURES, get_msec());

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

			camera->reset_identity();

			camera->rotate(Vector3(0, 1, 0), DEG_TO_RAD(theta));

			set_view_matrix(xform_data.view * (*camera));

			/*Render calls*/
			glActiveTexture(GL_TEXTURE0 + 3);
			glBindTexture(GL_TEXTURE_2D, game::engine::ovr_manager.get_ovr_render_target().get_depth_attachment());
			glActiveTexture(GL_TEXTURE0);
			set_unistate("st_shadow2D_tex", 3);
			set_unistate("st_shadow_matrix", shad_mat);
			game::engine::active_stage->render(STAGE_RENDER_SKY | STAGE_RENDER_SCENES |
				STAGE_RENDER_ENEMIES | STAGE_RENDER_PROJECTILES, get_msec());

			glActiveTexture(GL_TEXTURE0 + 3);
			glBindTexture(GL_TEXTURE_2D, game::engine::rtarg2.get_depth_attachment());
			glActiveTexture(GL_TEXTURE0);
			set_unistate("st_shadow2D_tex", 3);
			set_unistate("st_shadow_matrix", shad_mat2);

			game::engine::active_stage->render(STAGE_RENDER_GUNS, get_msec());
			game::engine::nuc_manager->render(RENDER_PSYS, get_msec());
			game::engine::active_stage->render(STAGE_RENDER_COCKPIT | STAGE_RENDER_HOLOMAP | STAGE_RENDER_CROSSHAIR, get_msec());
			/*-----------------------------------------------------------------------------------------*/
		}
	}

	is_visible = game::engine::ovr_manager.submit_ovr_frame();

	game::engine::ovr_manager.draw_ovr_mirror_texture(game::options::win_width, game::options::win_height);
}

void GameScreen::shadow_render_pass(bool ovr, long time)
{
	RenderTarget *rtarg;
	if (ovr){
		rtarg = &game::engine::ovr_manager.get_ovr_render_target();
	}
	else{
		rtarg = &game::engine::rtarg;
	}

	game::engine::rndr_pass_type = RNDR_PASS_SHADOW;

	Matrix4x4 orig_proj = get_projection_matrix();
	Matrix4x4 orig_view = get_view_matrix();

    Vector3 l_pos = Vector3(500, 500, -500);
	Vector3 l_target = -l_pos;

	glCullFace(GL_FRONT);

	rtarg->bind(RT_SHADOW);
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Matrix4x4 l_proj;

	l_proj.set_perspective(DEG_TO_RAD(6.0), 1.0, 600, 890);
	set_projection_matrix(l_proj);

	glViewport(0, 0, game::engine::rtarg.get_shad_tex_width(), game::engine::rtarg.get_shad_tex_height());

	Matrix4x4 l_view;
	l_view.set_lookat(l_pos, l_target, Vector3(0, 1, 0));
	set_view_matrix(l_view);

	shad_mat.reset_identity();
	shad_mat = l_proj * l_view;

	glColorMask(0, 0, 0, 0);

	game::engine::current_sdr = game::engine::depth_pass_sdr;
	game::engine::active_stage->render(STAGE_RENDER_ENEMIES, time);

	glColorMask(1, 1, 1, 1);
	rtarg->unbind();

	/*2nd shadow pass*/
	game::engine::rtarg2.bind(RT_SHADOW);
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	l_proj.reset_identity();
	l_proj.set_perspective(DEG_TO_RAD(0.4), 1.0, 600, 890);
	set_projection_matrix(l_proj);

	glViewport(0, 0, game::engine::rtarg2.get_shad_tex_width(), game::engine::rtarg2.get_shad_tex_height());

	l_view.reset_identity();
	l_view.set_lookat(l_pos, l_target, Vector3(0, 1, 0));
	set_view_matrix(l_view);

	shad_mat2.reset_identity();
	shad_mat2 = l_proj * l_view;

	glColorMask(0, 0, 0, 0);

	game::engine::active_stage->render(STAGE_RENDER_GUNS | STAGE_RENDER_COCKPIT, time);

	glColorMask(1, 1, 1, 1);
	game::engine::rtarg2.unbind();
	/*------------------------------------------------------------*/

	set_projection_matrix(orig_proj);
	set_view_matrix(orig_view);

	glViewport(0, 0, game::engine::rtarg.get_fb_width(), game::engine::rtarg.get_fb_height());
	glCullFace(GL_BACK);
}

void GameScreen::color_render_pass(long time)
{

	game::engine::active_stage->render(STAGE_RENDER_COCKPIT_SCRN_TEXTURES, time);

	game::engine::rndr_pass_type = RNDR_PASS_COLOR;
	game::engine::rtarg.bind(RT_COLOR_AND_DEPTH);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, game::engine::rtarg.get_depth_attachment());
	glActiveTexture(GL_TEXTURE0);
	set_unistate("st_shadow2D_tex", 3);
	set_unistate("st_shadow_matrix", shad_mat);

	game::engine::active_stage->render(STAGE_RENDER_SKY | STAGE_RENDER_SCENES |
		STAGE_RENDER_ENEMIES | STAGE_RENDER_PROJECTILES, time);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, game::engine::rtarg2.get_depth_attachment());
	glActiveTexture(GL_TEXTURE0);
	set_unistate("st_shadow2D_tex", 3);
	set_unistate("st_shadow_matrix", shad_mat2);

	XFormNode *hud = game::engine::active_stage->get_hud_ctrl_node_ptr();
	hud->set_matrix((*camera).inverse());
	hud->calc_matrix(time);

	game::engine::active_stage->render(STAGE_RENDER_GUNS, time);
	game::engine::nuc_manager->update(dt, time);
	game::engine::nuc_manager->render(RENDER_PSYS, time);
	game::engine::active_stage->render(STAGE_RENDER_COCKPIT | STAGE_RENDER_HOLOMAP | STAGE_RENDER_HUD | STAGE_RENDER_CROSSHAIR, time);

	game::engine::rtarg.unbind();
}

void GameScreen::draw_shad_tex()
{
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(game::engine::depth_dbg_sdr->get_id());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, game::options::win_width, game::options::win_height);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, game::engine::rtarg2.get_depth_attachment());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	set_unistate("st_shadow2D_tex", 0);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(-1, -1);
	glTexCoord2f(1, 0);
	glVertex2f(1, -1);
	glTexCoord2f(1, 1);
	glVertex2f(1, 1);
	glTexCoord2f(0, 1);
	glVertex2f(-1, 1);
	glEnd();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glUseProgram(0);
}

void GameScreen::render()
{
	if (game::options::ovr_enabled){
		render_ovr();
	}
	else{
		render_default();
	}
}

void GameScreen::on_mouse_down(int btn, int x, int y)
{
	BaseScreen::on_mouse_down(btn, x, y);

	if (btn == MOUSE_LEFT){
		game::engine::active_stage->set_shooting_state(true);
	}

	if (btn == MOUSE_RIGHT){
		WeaponSystem *w = game::engine::active_stage->get_weapon_system_ptr();
		w->set_coolant_usage(true);
	}
}

void GameScreen::on_mouse_up(int btn, int x, int y)
{
	BaseScreen::on_mouse_up(btn, x, y);

	if (btn == MOUSE_LEFT){
		game::engine::active_stage->set_shooting_state(false);
	}

	if (btn == MOUSE_RIGHT){
		WeaponSystem *w = game::engine::active_stage->get_weapon_system_ptr();
		w->set_coolant_usage(false);
	}
}

void GameScreen::on_double_click(int btn, int x, int y)
{

}

void GameScreen::on_move(int x, int y)
{
	theta += (float)x / 10.0;
	phi += (float)y / 10.0;

	if (theta > 360 || theta < -360)
		theta = 0;

	if (phi < -90)
		phi = -90;
	if (phi > 0)
		phi = 0;
}

void GameScreen::on_drag(int x, int y)
{
	BaseScreen::on_drag(x, y);

	theta += (float)x / 10.0;
	phi += (float)y / 10.0;

	if (theta > 360 || theta < -360)
		theta = 0;

	if (phi < -90)
		phi = -90;
	if (phi > 0)
		phi = 0;
}

void GameScreen::on_key_press(unsigned char key, int x, int y)
{
	switch (key){	
	case 32:
		if (game::engine::active_stage->has_tutorial_started()) {
			game::engine::active_stage->set_skip_tutorial_state(true);
		}
		break;
	}
}

void GameScreen::on_key_release(unsigned char key, int x, int y)
{

}

void GameScreen::reshape(int x, int y)
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
		}

		game::engine::update_rendering_quad_tc();
	}
}

void GameScreen::update()
{
	game::engine::active_stage->update(get_msec(), dt);
}

//DEBUG
static void render_axes()
{
	glUseProgram(0);
	glDisable(GL_LIGHTING);
	glLineWidth(10.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadTransposeMatrixf(get_projection_matrix().m[0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadTransposeMatrixf(get_view_matrix().m[0]);

	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(30000, 0, 0);
	glVertex3f(-30000, 0, 0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0, 1, 0);
	glVertex3f(0, -30000, 0);
	glVertex3f(0, 30000, 0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 30000);
	glVertex3f(0, 0, -30000);
	glEnd();

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 360; i++){
		float degInRad = DEG_TO_RAD(i);
		glVertex3f(cos(degInRad) * 10, sin(degInRad) * 10, 0);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	glColor3f(0, 1, 0);
	for (int i = 0; i < 360; i++){
		float degInRad = DEG_TO_RAD(i);
		glVertex3f(cos(degInRad) * 10, 0, sin(degInRad) * 10);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	glColor3f(1, 0, 0);
	for (int i = 0; i < 360; i++){
		float degInRad = DEG_TO_RAD(i);
		glVertex3f(0, cos(degInRad) * 10, sin(degInRad) * 10);
	}
	glEnd();

	glEnable(GL_LIGHTING);
}

//---

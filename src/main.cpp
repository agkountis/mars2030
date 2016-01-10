#include <assert.h>
#include "game.h"

/*SDL Window and GL Context*/
static SDL_Window *win;
static SDL_GLContext ctx;
/*----------------------------*/

bool init();
void display();
void update();
void reshape(int x, int y);

bool handle_event(SDL_Event *ev);
void toggle_hmd_fullscreen();
void cleanup();

//DEBUG
void draw_cube(int size_x, int size_y, int size_z, int normal_sign);

int main(int argc, char **argv)
{
	if (!init())
		return 1;


	while (!game::engine::exit_game){
		SDL_Event ev;

		while (SDL_PollEvent(&ev)){
            if (!handle_event(&ev)){
				break;
			}
		}

        update();
        reshape(game::options::win_width, game::options::win_height);
        display();
	}

	cleanup();

	return 0;
}

bool init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0){
		std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
		return false;
	}

	if (!game::engine::ovr_manager.init_ovr_library()) {
		return false;
	}

	/*Enable Double Buffering*/
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);

	/*Enable Vsync*/
    SDL_GL_SetSwapInterval(SDL_TRUE);

    if (!(win = SDL_CreateWindow("Mars2030", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE))){
        std::cerr << "Failed to create the Window!"<<SDL_GetError() << std::endl;
		return false;
    }

	if (!(ctx = SDL_GL_CreateContext(win))) {
		std::cerr << "Failed to initialize the OpenGL context!" << std::endl;
		return false;
    }

    SDL_GetWindowSize(win, &game::options::win_width, &game::options::win_height);

	glewInit();

	/*Enable Depth Testing*/
	glEnable(GL_DEPTH_TEST);

	/*Enable back face culling*/
	glEnable(GL_CULL_FACE);

	/*Enable Lighting*/
	glEnable(GL_LIGHTING);

	glEnable(GL_FRAMEBUFFER_SRGB);

	if (!game::pre_load_init())
		return false;

	if (!game::load("data/config/game_assets.config"))
		return false;

	if (!game::post_load_init())
		return false;

	return true;
}

void display()
{
    game::engine::scrn_manager->render();

    SDL_GL_SwapWindow(win);

	assert(glGetError() == GL_NO_ERROR);
}

void update()
{
	/*Get the window width/height*/
	SDL_GetWindowSize(win, &game::options::win_width, &game::options::win_height);

	if (game::options::fullscreen){
		SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else{
		SDL_SetWindowFullscreen(win, SDL_FALSE);
	}

    if(game::options::ovr_enabled){

		if (!game::engine::ovr_initialized){

			if (game::engine::ovr_manager.init_ovr_data(game::options::win_width, game::options::win_height)) {

				game::engine::ovr_initialized = true;

				// Turn off vsync to let the compositor do its magic
				SDL_GL_SetSwapInterval(SDL_FALSE);
			}
			else {
				std::cerr << "OVR initialization failed!!" << std::endl;
				game::options::ovr_enabled = false;
			}
		}
	}
	else{

		if (game::engine::ovr_initialized) {

			game::engine::ovr_manager.destroy_ovr_data();
			game::engine::ovr_initialized = false;

		}

		SDL_GL_SetSwapInterval(SDL_TRUE);

		game::options::ss_factor = 2.0;
	}

    switch(game::engine::warp_mouse){
    case true:
        SDL_SetRelativeMouseMode(SDL_TRUE);
        break;
    case false:
        SDL_SetRelativeMouseMode(SDL_FALSE);
        break;
    }

    game::engine::scrn_manager->calc_screen_dt();
    game::engine::scrn_manager->update();	
    game::engine::audio_manager->regulate_volumes();
}

void reshape(int x, int y)
{
    game::engine::scrn_manager->reshape(x, y);
}

bool handle_event(SDL_Event *ev)
{
    static int x, y;
    static int xrel,yrel;

    switch (ev->type){
    case SDL_MOUSEMOTION:
        /*Get the cursor position*/
        x = ev->motion.x;
        y = ev->motion.y;

        /*Get the cursor position relative to the window
         * (it represents the delta x and delta y.
         * Current cursor position - previous cursor position).*/
        xrel = ev->motion.xrel;
        yrel = ev->motion.yrel;
        if(game::engine::event_handler->get_mouse_btn_state() == BUTTON_PRESSED){
			switch (game::engine::warp_mouse){
			case true:
				game::engine::event_handler->on_drag(xrel, yrel);
				break;
			case false:
				game::engine::event_handler->on_drag(x, y);
			}
        }else{
            switch (game::engine::warp_mouse){
            case true:
                game::engine::event_handler->on_move(xrel, yrel);
                break;
            case false:
                game::engine::event_handler->on_move(x, y);
                break;
            }
        }
        break;

	case SDL_MOUSEBUTTONDOWN:
		game::engine::event_handler->set_mouse_btn_state(BUTTON_PRESSED);
		game::engine::event_handler->on_mouse_down(ev->button.button - 1, x, y);
		break;

    case SDL_MOUSEBUTTONUP:
        game::engine::event_handler->set_mouse_btn_state(BUTTON_RELEASED);
        game::engine::event_handler->on_mouse_up(ev->button.button - 1, x, y);
        break;

    case SDL_KEYDOWN:
        game::engine::event_handler->on_key_press(ev->key.keysym.sym, x, y);
        break;

    case SDL_KEYUP:
        game::engine::event_handler->on_key_release(ev->key.keysym.sym, x, y);
        if(!game::engine::exit_game)
            return false;
        break;
    }

	return true;
}

void cleanup()
{
	SDL_GL_DeleteContext(ctx);
	SDL_DestroyWindow(win);
	SDL_Quit();

    game::cleanup();

	game::engine::ovr_manager.shutdown_ovr_library();
}

void draw_cube(int size_x, int size_y, int size_z, int normal_sign)
{
	glBegin(GL_QUADS);

	/*front face*/
	glNormal3f(0, 0, 1 * normal_sign);
	glVertex3f(-size_x, -size_y, size_z);
	glVertex3f(size_x, -size_y, size_z);
	glVertex3f(size_x, size_y, size_z);
	glVertex3f(-size_x, size_y, size_z);

	/*back face*/
	glNormal3f(0, 0, -1 * normal_sign);
	glVertex3f(-size_x, -size_y, -size_z);
	glVertex3f(-size_x, size_y, -size_z);
	glVertex3f(size_x, size_y, -size_z);
	glVertex3f(size_x, -size_y, -size_z);

	/*right face*/
	glNormal3f(1 * normal_sign, 0, 0);
	glVertex3f(size_x, -size_y, size_z);
	glVertex3f(size_x, -size_y, -size_z);
	glVertex3f(size_x, size_y, -size_z);
	glVertex3f(size_x, size_y, size_z);

	/*left face*/
	glNormal3f(-1 * normal_sign, 0, 0);
	glVertex3f(-size_x, -size_y, -size_z);
	glVertex3f(-size_x, -size_y, size_z);
	glVertex3f(-size_x, size_y, size_z);
	glVertex3f(-size_x, size_y, -size_z);

	/*top face*/
	glNormal3f(0, 1 * normal_sign, 0);
	glVertex3f(size_x, size_y, size_z);
	glVertex3f(size_x, size_y, -size_z);
	glVertex3f(-size_x, size_y, -size_z);
	glVertex3f(-size_x, size_y, size_z);

	/*bottom face*/
	glNormal3f(0, -1 * normal_sign, 0);
	glNormal3f(0, 1 * normal_sign, 0);
	glVertex3f(size_x, -size_y, size_z);
	glVertex3f(-size_x, -size_y, size_z);
	glVertex3f(-size_x, -size_y, -size_z);
	glVertex3f(size_x, -size_y, -size_z);


	glEnd();
}

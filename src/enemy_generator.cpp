#include "enemy_generator.h"
#include "game.h"

//SpawnmapData struct constructor----------------------
SpawnmapData::SpawnmapData()
{
    path = "";
    height = 0;
    width = 0;
    spawnmap_pxl = NULL;
}
//-----------------------------------------------------

EnemyGenerator::EnemyGenerator()
{
    amount = 0;
    type = "";
    generation_rate = 0;
    enemies_to_generate = 0;
}

EnemyGenerator::EnemyGenerator(const std::string &type, int amount, float generation_rate)
{
    this->type = type;
    this->amount = amount;
    this->generation_rate = generation_rate;
    enemies_to_generate = 0;
}

EnemyGenerator::~EnemyGenerator()
{

}

/*Private functions*/
void EnemyGenerator::gen_from_spawnmap(float dt, long time)
{
	//TODO
}

void EnemyGenerator::gen_from_spherical(float dt, long time)
{
	Enemy *en = NULL;

	enemies_to_generate += generation_rate * dt;
	int num_gen = enemies_to_generate;
	enemies_to_generate -= num_gen;

	for (unsigned int i = 0; i < num_gen; i++){
		if (wave){
			en = new Enemy;

			unsigned int idx = game::assets::enemy_idx_by_name[type];
			EnemyTemplate *tmpl = game::assets::enemy_templates[idx];
			en->set_enemy_template(tmpl);

			float rand_x_factor = ((float)rand() / (float)RAND_MAX) * sph_data.spawn_radius * 2
				- sph_data.spawn_radius;

			float rand_y_factor = ((float)rand() / (float)RAND_MAX) * sph_data.spawn_radius * 2
				- sph_data.spawn_radius;

			float rand_z_factor = ((float)rand() / (float)RAND_MAX) * sph_data.spawn_radius * 2
				- sph_data.spawn_radius;
			Vector3 rand_vec = Vector3(rand_x_factor, rand_y_factor, rand_z_factor);
			Vector3 pos = game::utils::spherical_to_cartesian_z_ref(DEG_TO_RAD(sph_data.phi), DEG_TO_RAD(sph_data.theta), sph_data.radius);
			pos += rand_vec;
			en->set_velocity(-pos.normalized());

			en->set_position(pos);
			en->init();

			Holomap *map = game::engine::active_stage->get_holomap_ptr();
			Holonode *holonode = new Holonode;
			holonode->en = en;
			map->add_node(holonode);

			wave->add_enemy(en);
		}
		amount--;
	}
}
/*------------------------------------------------------*/

void EnemyGenerator::set_type(const std::string &type)
{
    this->type = type;
}

void EnemyGenerator::set_amount(int amount)
{
    this->amount = amount;
}

void EnemyGenerator::set_spherical_spawn_data(const SphericalSpawnData &sph_data)
{
	this->sph_data = sph_data;
}

void EnemyGenerator::set_generation_rate(float gen_rate)
{
	generation_rate = gen_rate;
}

void EnemyGenerator::assign_wave(Wave *wv)
{
	wave = wv;
}

const std::string& EnemyGenerator::get_type() const
{
    return type;
}

int EnemyGenerator::get_amount() const
{
    return amount;
}

float EnemyGenerator::get_generation_rate() const
{
    return generation_rate;
}

SpawnmapData* EnemyGenerator::get_spawnmap_ptr()
{
    return &spawnmap;
}

float EnemyGenerator::get_spawnmap_pxl(int x, int y) const
{
	if(!spawnmap.spawnmap_pxl) {
		return 0.0;
	}
    int px= x < 0 ? 0 : ( x >= spawnmap.width ? spawnmap.width - 1 : x);
    int py= y < 0 ? 0 : ( y >= spawnmap.height ? spawnmap.height - 1 : y);

    return spawnmap.spawnmap_pxl[py * spawnmap.width + px];
}

bool EnemyGenerator::load_spawnmap_img(const char *fname)
{
    spawnmap.spawnmap_pxl = (float*)img_load_pixels(fname, &spawnmap.width, &spawnmap.height, IMG_FMT_GREYF);

    if(!spawnmap.spawnmap_pxl)
        return false;
    return true;
}

void EnemyGenerator::generate(unsigned int gen_method, float dt, long time)
{
	if (wave){
		switch (gen_method){
		case SPAWNMAP_GEN:
			gen_from_spawnmap(dt, time);
			break;
        case SPHERICAL_GEN:
			gen_from_spherical(dt, time);
			break;
		}
	}
	else{
		std::cerr << "Can't generate enemies! Assign a wave pointer to the generator!" << std::endl;
	}

}

bool EnemyGenerator::finished_generating() const
{
    return amount == 0;
}

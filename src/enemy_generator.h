#ifndef SUB_WAVE_H
#define SUB_WAVE_H
#include <string>
#include "vmath.h"
#include "enemy.h"

enum SpawnMethod {
	SPAWNMAP_GEN = 0, SPHERICAL_GEN
};

struct SpawnmapData
{
    std::string path;
    int width;
    int height;
    float *spawnmap_pxl;

    SpawnmapData();
};

struct SphericalSpawnData
{
	float radius;
	float phi;
	float theta;

	float spawn_radius;
};

class EnemyGenerator
{
private:
    std::string type;
    int amount;
    SpawnmapData spawnmap;
	SphericalSpawnData sph_data;

    float generation_rate;
    float enemies_to_generate;

	void gen_from_spawnmap(float dt, long time = 0);
	void gen_from_spherical(float dt, long time = 0);

	/*The wave this generator is assigned to.*/
	Wave *wave;

public:
    EnemyGenerator();
    EnemyGenerator(const std::string &type, int amount, float generation_rate);
    ~EnemyGenerator();

    void set_type(const std::string &type);
    void set_amount(int amount);
	void set_spherical_spawn_data(const SphericalSpawnData &sph_data);
	void set_generation_rate(float gen_rate);
	void assign_wave(Wave *wv);

    const std::string &get_type() const;
    int get_amount() const;
    float get_generation_rate() const;
    SpawnmapData *get_spawnmap_ptr();

    float get_spawnmap_pxl(int x, int y) const;

    bool load_spawnmap_img(const char *fname);

    void generate(unsigned int gen_method, float dt, long time = 0);
    bool finished_generating() const;
};

#endif // SUB_WAVE_H

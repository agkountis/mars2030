#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <assimp/scene.h>
#include "vmath.h"
#include "texture.h"

enum TexType {
    TEX_DIFFUSE = 0,
	TEX_SPECULAR,
	TEX_NORMAL,

	NUM_TEX_TYPES
};

class Material
{
public:
	Vector4 diffuse; //color
	Vector4 specular; //color
	float alpha;
	float shininess;

     //put this globaly in the game_assets
    goatgfx::Texture *textures[NUM_TEX_TYPES];

	Material();
	~Material();
	void load_material(const aiMaterial *m);
	void setup_material();
};

#endif //MATERIAL_H_

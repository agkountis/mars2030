#include "material.h"
#include "unistate.h"
#include "game.h"


static const char *get_tex_fname(const char* fname);

Material::Material()
{
    for(int i = 0 ; i < NUM_TEX_TYPES ; i++)
	{
        textures[i] = 0;
	}

    this->diffuse = Vector4(1, 1, 1, 1);
    this->alpha = 1;
    this->specular = Vector4(1, 1, 1, 1);
    this->shininess = 60.0;
}

Material::~Material()
{
    /*for(int i = 0 ; i < NUM_TEX_TYPES ; i++) {
        if(textures[i])
            delete textures[i];
    }*/
}

void Material::load_material(const aiMaterial *m)
{
	aiColor4D col;

	//get the diff color.
	aiGetMaterialColor(m,AI_MATKEY_COLOR_DIFFUSE,&col);
	diffuse.x=col.r;
	diffuse.y=col.g;
	diffuse.z=col.b;
	diffuse.w=col.a;

	//get the spec color.
	aiGetMaterialColor(m,AI_MATKEY_COLOR_SPECULAR,&col);
	specular.x=col.r;
	specular.y=col.g;
	specular.z=col.b;
	specular.w=col.a;


	//get the shininess.
	aiGetMaterialFloat(m,AI_MATKEY_SHININESS,(float*)&shininess);

	//get the opacity.
	aiGetMaterialFloat(m,AI_MATKEY_OPACITY,(float*)&alpha);

	aiString str; //for the Texture filenames.

	//get the diffuse textures.
	if(m->GetTextureCount(aiTextureType_DIFFUSE) != 0)
	{
		for(unsigned int i=0;i<m->GetTextureCount(aiTextureType_DIFFUSE);i++)
		{
			m->GetTexture(aiTextureType_DIFFUSE,i,&str);
            textures[TEX_DIFFUSE]=game::engine::texset->get(get_tex_fname(str.C_Str()));
			if(!textures[TEX_DIFFUSE])
				std::cout<<"ERROR: Diffuse map texture loading failed!"<<std::endl;
		}
	}

	//get the specular textures.
	if(m->GetTextureCount(aiTextureType_SPECULAR) !=0)
	{
		for(unsigned int i=0;i<m->GetTextureCount(aiTextureType_SPECULAR);i++)
		{
			m->GetTexture(aiTextureType_SPECULAR,i,&str);
            textures[TEX_SPECULAR]=game::engine::texset->get(get_tex_fname(str.C_Str()));
			if(!textures[TEX_SPECULAR])
				std::cout<<"ERROR: Specular map texture loading failed!"<<std::endl;
		}
	}

	if(m->GetTextureCount(aiTextureType_NORMALS) !=0)
	{
		for(unsigned int i=0;i<m->GetTextureCount(aiTextureType_NORMALS);i++)
		{
			m->GetTexture(aiTextureType_NORMALS,i,&str);
            textures[TEX_NORMAL]=game::engine::texset->get(get_tex_fname(str.C_Str()));
			if(!textures[TEX_NORMAL])
				std::cout<<"ERROR: Normal map texture loading failed!"<<std::endl;
		}
	}
}

void Material::setup_material()
{
	set_unistate("st_mtl_diffuse",diffuse);
	set_unistate("st_mtl_specular",specular);
	set_unistate("st_mtl_shininess",shininess);
	set_unistate("st_mtl_alpha",alpha);

	if(textures[TEX_DIFFUSE])
	{
        textures[TEX_DIFFUSE]->bind(TEX_DIFFUSE);
        set_unistate("st_diff_tex", TEX_DIFFUSE);
	}
    else
    {
        game::assets::diff_dummy->bind(TEX_DIFFUSE);
        set_unistate("st_diff_tex", TEX_DIFFUSE);
    }

	if(textures[TEX_SPECULAR])
	{
		textures[TEX_SPECULAR]->bind(TEX_SPECULAR);
        set_unistate("st_spec_tex", TEX_SPECULAR);
	}
    else
    {
        game::assets::spec_dummy->bind(TEX_SPECULAR);
        set_unistate("st_spec_tex", TEX_SPECULAR);
    }

	if(textures[TEX_NORMAL])
	{
        textures[TEX_NORMAL]->bind(TEX_NORMAL);
        set_unistate("st_norm_map_tex", TEX_NORMAL);
	}
    else
    {
        game::assets::norm_dummy->bind(TEX_NORMAL);
        set_unistate("st_norm_map_tex", TEX_NORMAL);
    }
}

static const char *get_tex_fname(const char* fname)
{
	const char *res=fname;
	const char *last_slash,*last_back_slash;

	last_back_slash=strrchr(res,'\\');
	last_slash=strrchr(res,'/');

	if(last_back_slash)
	{
		res=last_back_slash+1;
	}
	if(last_slash)
	{
		res=last_slash+1;
	}

	return res;
}


#ifndef NUC_RENDERER_H
#define NUC_RENDERER_H
#include <list>
#include "sdrman.h"
#include "unistate.h"
#include "nuc_particle.h"
#include "texture.h"

#define VBO_COUNT 2
#define PARTICLE_POOL_SIZE 250



class NucRenderer
{
protected:
	ShaderProg *sdr;
	unsigned int vbos[VBO_COUNT];

    unsigned int actv_vbo;
    unsigned int pool_counter;

    ParticleAttributes p_attr_array[PARTICLE_POOL_SIZE];

    /*An enumeration for the particle attributes.
     *Primarily used when rendering with VBOs etc.*/
    enum p_attribs {
        P_ATTR_POSITION = 0, P_ATTR_COLOR, P_ATTR_ALPHA, P_ATTR_SIZE
    };

public:
	NucRenderer();
	virtual ~NucRenderer();

    void set_shader_program(ShaderProg *sdr);
    ShaderProg *get_shader_program() const;

    virtual void init() = 0;
    virtual void bind_attributes() = 0;

    virtual void pre_render_ops() = 0; /*pre rendering operations*/
    virtual void render(const std::list<NucParticle*> &particles
                        ,goatgfx::Texture *tex, float size) = 0; /*actual rendering*/
    virtual void post_render_ops() = 0; /*post rendering operations*/
};

#endif //NUC_RENDERER_H

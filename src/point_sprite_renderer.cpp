#include "point_sprite_renderer.h"
#include "game.h"
#include <assert.h>

PointSpriteRenderer::PointSpriteRenderer()
{

}

PointSpriteRenderer::~PointSpriteRenderer()
{

}

void PointSpriteRenderer::flush_particles()
{
	glEnable(GL_POINT_SPRITE_ARB);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[actv_vbo]);

    glBufferSubData(GL_ARRAY_BUFFER, 0, pool_counter * sizeof(ParticleAttributes),
                    p_attr_array);

    glEnableVertexAttribArray(P_ATTR_POSITION);
    glVertexAttribPointer(P_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleAttributes), 0);

    glEnableVertexAttribArray(P_ATTR_COLOR);
    glVertexAttribPointer(P_ATTR_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleAttributes), (void*)(3*sizeof(float)));

    glEnableVertexAttribArray(P_ATTR_SIZE);
    glVertexAttribPointer(P_ATTR_SIZE, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleAttributes), (void*)(7*sizeof(float)));

    //draw call
    glDrawArrays(GL_POINTS, 0, pool_counter);

    glDisableVertexAttribArray(P_ATTR_POSITION);
    glDisableVertexAttribArray(P_ATTR_COLOR);
    glDisableVertexAttribArray(P_ATTR_SIZE);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if(actv_vbo != VBO_COUNT - 1)
    {
        actv_vbo++;
    }
    else
    {
        actv_vbo = 0;
    }

    pool_counter = 0;

	glDisable(GL_POINT_SPRITE_ARB);
}

void PointSpriteRenderer::init()
{
    for(unsigned int i = 0 ; i < VBO_COUNT ; i++)
    {
        glGenBuffers(1, &vbos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, PARTICLE_POOL_SIZE * sizeof(ParticleAttributes),
                     NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    bind_attributes();
}

void PointSpriteRenderer::bind_attributes()
{
    sdr->set_attrib_location("attr_pos", 0);
    sdr->set_attrib_location("attr_col", 1);
    sdr->set_attrib_location("attr_size", 3);
}

void PointSpriteRenderer::pre_render_ops()
{

}

void PointSpriteRenderer::render(const std::list<NucParticle *> &particles, goatgfx::Texture *tex, float size)
{
    /*if the 1st vbo is not initialized the 2nd one won't be either
     *so initialize them both!*/
    if(!vbos[0])
        init();

    if(!particles.empty()){
        if(tex){
            tex->bind(0);
            set_unistate("st_diff_tex", 0);
        }

        glEnable(GL_PROGRAM_POINT_SIZE);
        glDepthMask(0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        sdr->bind();

        std::list<NucParticle*>::const_iterator it = particles.begin();

        while(it != particles.end()){
            NucParticle *p = *it;

            p_attr_array[pool_counter] = p->get_particle_attributes();
            pool_counter++;

            if(pool_counter == PARTICLE_POOL_SIZE){
                flush_particles();
            }

			it++;
        }

        if(pool_counter)
            flush_particles();

        glDisable(GL_BLEND);
        glDepthMask(1);
    }

}

void PointSpriteRenderer::post_render_ops()
{

}

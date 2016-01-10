#include "nuc_renderer.h"

NucRenderer::NucRenderer()
{
    sdr = NULL;
    actv_vbo = 0;
    pool_counter = 0;

    for(unsigned int i = 0 ; i < VBO_COUNT ; i++)
    {
        vbos[i] = 0;
    }
}

NucRenderer::~NucRenderer()
{
    for(unsigned int i = 0 ; i < VBO_COUNT ; i++) {
        glDeleteBuffers(1, &vbos[i]);
    }
}

void NucRenderer::set_shader_program(ShaderProg *sdr)
{
    this->sdr = sdr;
}

ShaderProg* NucRenderer::get_shader_program() const
{
    return sdr;
}

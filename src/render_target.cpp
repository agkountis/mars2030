#include "render_target.h"
#include "game.h"

RenderTarget::RenderTarget()
{
    fbo = 0;
	shadow_fbo = 0;
    depth_renderbuffer = 0;
    dummy_renderbuffer = 0;

	color_attachment = 0;
	depth_attachment = 0;


    fb_width = 0;
    fb_height = 0;
	fb_tex_width = 0;
	fb_tex_height = 0;
	shad_tex_width = 0;
	shad_tex_height = 0;
}

RenderTarget::~RenderTarget()
{
	if (fbo)
		glDeleteFramebuffers(1, &fbo);
	if (shadow_fbo)
		glDeleteFramebuffers(1, &shadow_fbo);
	if (depth_attachment)
		glDeleteRenderbuffers(1, &depth_renderbuffer);

	if (color_attachment)
		glDeleteTextures(1, &color_attachment);

	if (depth_attachment)
		glDeleteTextures(1, &depth_attachment);
}

bool RenderTarget::create(unsigned int rt_mask)
{
	if (rt_mask &(1 << RT_COLOR_AND_DEPTH)){
		/*If the FBO hasn't been created, create now*/
		if (!fbo){
			glGenFramebuffers(1, &fbo);
		}

		fb_tex_width = game::utils::next_pow2(fb_width);
		fb_tex_height = game::utils::next_pow2(fb_height);

		/*If no texture for drawing color has been assigned
		 * create a new texture for the color attachment of the fbo*/
		if (!color_attachment){
			glGenTextures(1, &color_attachment);

			/*initialize the color attachment texture*/
			glBindTexture(GL_TEXTURE_2D, color_attachment);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			/*Create the texture 2d image*/
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fb_tex_width, fb_tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		}

		/*bind the framebuffer to perform operations on it*/
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		/*Attach the color attachment texture to the fbo*/
		glBindTexture(GL_TEXTURE_2D, color_attachment);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment, 0);

		/*Create a Renderbuffer for depth storage (used for depth testing)*/
		glGenRenderbuffers(1, &depth_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fb_tex_width, fb_tex_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
			std::cerr << "Incomplete Framebuffer!" << std::endl;
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		std::cout << "Created render target: " << fb_width << "x" << fb_height << "(texture size:" <<
			fb_tex_width << "x" << fb_tex_height << ")" << std::endl;
	}

	if (rt_mask &(1 << RT_SHADOW)){
		if (!depth_attachment && (!shad_tex_width || !shad_tex_height)){
			std::cerr << "No dimensions for depth storage texture have been assigned or one of them is 0."<< 
				"Shadow fbo not created!" << std::endl;
			return false;
		}
		glGenFramebuffers(1, &shadow_fbo);

		if (!depth_attachment){
			glGenTextures(1, &depth_attachment);

			/*Create and parametrize the depth storage texture*/
			glBindTexture(GL_TEXTURE_2D, depth_attachment);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shad_tex_width, shad_tex_height,
				0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		}

		/*Bind the framebuffer to perform operations on it.*/
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, 0);

        glGenRenderbuffers(1, &dummy_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, dummy_renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, shad_tex_width, shad_tex_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, dummy_renderbuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
			std::cerr << "Incomplete Framebuffer!" << std::endl;
			return false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		std::cout << "Created shadow render target: " << fb_width << "x" << fb_height << "(texture size:" <<
			shad_tex_width << "x" << shad_tex_height << ")" << std::endl;
	}

	return true;
}

void RenderTarget::bind(RenderTargetType rt_type)
{
	if (rt_type == RT_COLOR_AND_DEPTH){
		if (!fbo){
			std::cout << "Color framebuffer not created! Creating color framebuffer..." << std::endl;
			create(RT_COLOR_AND_DEPTH);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	if (rt_type == RT_SHADOW){
		if (!shadow_fbo){
			std::cout << "Depth framebuffer not created! Creating depth framebuffer..." << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
	}
}

void RenderTarget::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::set_color_attachment(unsigned int tex)
{
    color_attachment = tex;
}

void RenderTarget::set_and_assign_color_attachment(unsigned int tex)
{
	color_attachment = tex;
	
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::set_depth_attachment(unsigned int tex)
{
    depth_attachment = tex;
}

void RenderTarget::set_fb_width(unsigned int width)
{
    fb_width = width;
}

void RenderTarget::set_fb_height(unsigned int height)
{
    fb_height = height;
}

void RenderTarget::set_shad_tex_width(unsigned int width)
{
	shad_tex_width = width;
}

void RenderTarget::set_shad_tex_height(unsigned int height)
{
	shad_tex_height = height;
}

void RenderTarget::set_fb_tex_width(unsigned int width)
{
    fb_tex_width = width;
}

void RenderTarget::set_fb_tex_height(unsigned int height)
{
    fb_tex_height = height;
}

unsigned int RenderTarget::get_color_attachment() const
{
    return color_attachment;
}

unsigned int RenderTarget::get_depth_attachment() const
{
    return depth_attachment;
}

unsigned int RenderTarget::get_fb_width() const
{
    return fb_width;
}

unsigned int RenderTarget::get_fb_height() const
{
    return fb_height;
}

unsigned int RenderTarget::get_fb_tex_width() const
{
    return fb_tex_width;
}

unsigned int RenderTarget::get_fb_tex_height() const
{
    return fb_tex_height;
}

unsigned int RenderTarget::get_shad_tex_width() const
{
	return shad_tex_width;
}

unsigned int RenderTarget::get_shad_tex_height() const
{
	return shad_tex_height;
}

void RenderTarget::resize_render_target(int tex_width, int tex_height)
{
	if (color_attachment){
		glBindTexture(GL_TEXTURE_2D, color_attachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else{
		std::cerr << "The color attachment does not exist. Cannot resize the texture!!" << std::endl;
	}

    if(depth_renderbuffer){
        glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, tex_width, tex_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }else{
        std::cerr<<"The depth attachment does not exist. Cannot resize the renderbuffer!"<<std::endl;
    }
}

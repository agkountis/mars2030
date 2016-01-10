#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

enum RenderTargetType{
	RT_COLOR_AND_DEPTH = 1, RT_SHADOW
};

#define RT_ALL 0xffffffff
#define RT_COLOR_AND_DEPTH_MASK (1 << RT_COLOR_AND_DEPTH)
#define RT_SHADOW_MASK (1 << RT_SHADOW)

class RenderTarget
{
private:
    /*Framebuffers*/
    unsigned int fbo;
    unsigned int depth_renderbuffer;
    unsigned int dummy_renderbuffer;
	unsigned int shadow_fbo;

    /*Textures*/
    unsigned int color_attachment;
    unsigned int depth_attachment;

    /*Framebuffer(Window) dimensions*/
    unsigned int fb_width;
    unsigned int fb_height;

    /*Actual attachment dimensions*/
    unsigned int fb_tex_width;
    unsigned int fb_tex_height;

	/*Depth texture dimensions*/
	unsigned int shad_tex_width;
	unsigned int shad_tex_height;

public:
    RenderTarget();
    ~RenderTarget();

    bool create(unsigned int rt_mask);
    void bind(RenderTargetType rt_type);
    void unbind();
    void set_color_attachment(unsigned int tex);
	void set_and_assign_color_attachment(unsigned int tex);
    void set_depth_attachment(unsigned int tex);
    void set_fb_width(unsigned int width);
    void set_fb_height(unsigned int height);
    void set_fb_tex_width(unsigned int width);
    void set_fb_tex_height(unsigned int height);
	void set_shad_tex_width(unsigned int width);
	void set_shad_tex_height(unsigned int height);

    unsigned int get_color_attachment() const;
    unsigned int get_depth_attachment() const;
    unsigned int get_fb_width() const;
    unsigned int get_fb_height() const;
    unsigned int get_fb_tex_width() const;
    unsigned int get_fb_tex_height() const;
	unsigned int get_shad_tex_width() const;
	unsigned int get_shad_tex_height() const;

    void resize_render_target(int tex_width, int tex_height);
};

#endif // RENDER_TARGET_H

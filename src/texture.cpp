#include <math.h>
#include "texture.h"
#include "image.h"
#include "opengl.h"
#include "imago2.h"
#include "logger.h"
#include "datapath.h"

using namespace goatgfx;

static int glifmt_from_ifmt(unsigned int ifmt);
static int glfmt_from_ifmt(unsigned int ifmt);
static int gltype_from_ifmt(unsigned int ifmt);

static int glifmt_from_imgfmt(Image::Format fmt);

static unsigned int cur_target[8] = {
	GL_TEXTURE_2D, GL_TEXTURE_2D, GL_TEXTURE_2D, GL_TEXTURE_2D,
	GL_TEXTURE_2D, GL_TEXTURE_2D, GL_TEXTURE_2D, GL_TEXTURE_2D
};

namespace goatgfx {

void set_texture(Texture *tex, int tunit)
{
	if(tex) {
		tex->bind(tunit);
	} else {
		glActiveTexture(GL_TEXTURE0 + tunit);
		glBindTexture(cur_target[tunit], 0);
		glActiveTexture(GL_TEXTURE0);
	}
}

Texture *load_texture(const char *fname)
{
	TextureCube *texcube = new TextureCube;
	if(texcube->load(fname)) {
		return texcube;
	}
	delete texcube;

	Texture2D *tex = new Texture2D;
	if(tex->load(fname)) {
		return tex;
	}
	delete tex;
	return 0;
}

}	// namespace goatgfx

Texture::Texture()
{
	target = 0;
	sz[0] = sz[1] = sz[2] = 0;
	texfmt = 0;

	glGenTextures(1, &id);
}

Texture::~Texture()
{
	if(id) {
		glDeleteTextures(1, &id);
	}
}

void Texture::set_wrapping(unsigned int wrap)
{
	if(!target) {
		return;
	}

	glBindTexture(target, id);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
}

void Texture::set_filtering(unsigned int filt)
{
	unsigned int mag_filter;

	if(!target) {
		return;
	}

	switch(filt) {
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_LINEAR:
		mag_filter = GL_LINEAR;
		break;

	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		mag_filter = GL_NEAREST;
		break;

	default:
		mag_filter = filt;
	}

	set_filtering(filt, mag_filter);
}

void Texture::set_filtering(unsigned int min_filt, unsigned int mag_filt)
{
	glBindTexture(target, id);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filt);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filt);
}

unsigned int Texture::get_format() const
{
	return texfmt;
}

int Texture::get_size(int dim) const
{
	if(dim < 0 || dim >= 3) {
		return 0;
	}
	return sz[dim];
}

unsigned int Texture::get_id() const
{
	return id;
}

void Texture::set_id(unsigned int id)
{
	if (id)
		glDeleteTextures(1, &this->id);
	this->id = id;
}

void Texture::bind(int tex_unit) const
{
	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(target, id);
	glActiveTexture(GL_TEXTURE0);

	cur_target[tex_unit] = target;
}


// ---- Texture2D ----

Texture2D::Texture2D()
{
	target = GL_TEXTURE_2D;
}

void Texture2D::create(int xsz, int ysz, unsigned int ifmt)
{
	int fmt = glfmt_from_ifmt(ifmt);
	int type = gltype_from_ifmt(ifmt);

	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, glifmt_from_ifmt(ifmt), xsz, ysz, 0, fmt, type, 0);
	CHECKGLERR;
	sz[0] = xsz;
	sz[1] = ysz;
	texfmt = ifmt;
}

void Texture2D::set_image(const Image &img, int idx)
{
	texfmt = glifmt_from_imgfmt(img.get_format());
	unsigned int fmt = glfmt_from_ifmt(texfmt);
	unsigned int type = gltype_from_ifmt(texfmt);

	sz[0] = img.get_width();
	sz[1] = img.get_height();

	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#ifdef __GLEW_H__
	if(GLEW_SGIS_generate_mipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, texfmt, sz[0], sz[1], 0, fmt, type, img.get_pixels());
#ifdef __GLEW_H__
	} else {
		gluBuild2DMipmaps(GL_TEXTURE_2D, texfmt, sz[0], sz[1], fmt, type, img.get_pixels());
	}
#endif

#ifdef GL_ES_VERSION_2_0
	glGenerateMipmap(GL_TEXTURE_2D);
#endif
}

bool Texture2D::load(const char *fname)
{
	Image img;
	if(!img.load(fname)) {
		error_log("failed to load 2D texture: %s\n", fname);
		return false;
	}
	set_image(img);

	info_log("loaded 2D texture: %s\n", fname);
	return true;
}

bool Texture2D::save(const char *fname) const
{
#ifndef GL_ES_VERSION_2_0
	unsigned char *pixels = new unsigned char[sz[0] * sz[1] * 4];

	glBindTexture(GL_TEXTURE_2D, id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	if(img_save_pixels(fname, pixels, sz[0], sz[1]) == -1) {
		error_log("failed to save 2D texture: %s\n", fname);
		delete [] pixels;
		return false;
	}

	info_log("saved 2D texture: %s\n", fname);
	delete [] pixels;
	return true;
#else
	return false;	// TODO
#endif
}

// ---- TextureCube ----
static unsigned int cube_faces[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

TextureCube::TextureCube()
{
	target = GL_TEXTURE_CUBE_MAP;
}

void TextureCube::create(int xsz, int ysz, unsigned int ifmt)
{
	if(xsz != ysz) {
		error_log("trying to create cubemap with different width and height (%dx%d)\n", xsz, ysz);
		return;
	}

	texfmt = ifmt;

	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for(int i=0; i<6; i++) {
		glTexImage2D(cube_faces[i], 0, ifmt, xsz, ysz, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}
}

void TextureCube::set_image(const Image &img, int idx)
{
	texfmt = glifmt_from_imgfmt(img.get_format());
	unsigned int fmt = glfmt_from_ifmt(texfmt);
	unsigned int type = gltype_from_ifmt(texfmt);

	sz[0] = img.get_width();
	sz[1] = img.get_height();

	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(cube_faces[idx], 0, texfmt, sz[0], sz[1], 0, fmt, type, img.get_pixels());
}

bool TextureCube::load(const char *fname)
{
	static const float one_third = 1.0 / 3.0;
	static const float two_thirds = 2.0 / 3.0;
	static const float hcross[2][6] = {
		{0.5, 0.0, 0.25, 0.25, 0.25, 0.75}, {one_third, one_third, 0.0, two_thirds, one_third, one_third} };
	static const float vcross[2][6] = {
		{two_thirds, 0.0, one_third, one_third, one_third, one_third}, {0.25, 0.25, 0.0, 0.5, 0.25, 0.75} };
	static const float hsix[2][6] = {
		{0.0, 0.0, one_third, one_third, two_thirds, two_thirds}, {0.0, 0.5, 0.0, 0.5, 0.0, 0.5} };

	Image img;
	if(!img.load(fname)) {
		return false;
	}

	int xsz = img.get_width();
	int ysz = img.get_height();

	if(xsz / 4 == ysz / 3) {
		// horizontal cross, assume the vertical bit is center-left
		return load_multi(img, hcross[0], hcross[1], xsz / 4);
	}
	if(xsz / 3 == ysz / 4) {
		// vertical cross, assume the horizontal bit is center-top (180-rotated image 5)
		return load_multi(img, vcross[0], vcross[1], ysz / 4, (1 << 5));
	}
	if(xsz / 3 == ysz / 2) {
		// horizontal sixpack
		return load_multi(img, hsix[0], hsix[1], ysz / 2);
	}

	return false;
}

bool TextureCube::save(const char *fname) const
{
	return false;	// TODO
}

bool TextureCube::load_multi(const Image &img, const float *xoffsets, const float *yoffsets, float sz,
		unsigned int rotmask)
{
	for(int i=0; i<6; i++) {
		Image face;

		int xoffs = xoffsets[i] * img.get_width();
		int yoffs = yoffsets[i] * img.get_height();

		if(!face.set_pixels(sz, sz, img.get_pixels(), xoffs, yoffs, img.get_width(), img.get_format())) {
			return false;
		}

		if(rotmask & (1 << i)) {
			face.rotate_180();
		}
		set_image(face, i);
	}
	return true;
}

static int glifmt_from_ifmt(unsigned int ifmt)
{
#ifdef GL_ES_VERSION_2_0
	switch(ifmt) {
	case GL_LUMINANCE16F:
	case GL_LUMINANCE32F:
		ifmt = GL_LUMINANCE;
		break;

	case GL_RGB16F:
	case GL_RGB32F:
		ifmt = GL_RGB;
		break;

	case GL_RGBA16F:
	case GL_RGBA32F:
		ifmt = GL_RGBA;
		break;

	default:
		break;
	}
#endif
	return ifmt;	// by default just pass it through...
}

static int glfmt_from_ifmt(unsigned int ifmt)
{
	switch(ifmt) {
    case GL_LUMINANCE16F_ARB:
    case GL_LUMINANCE32F_ARB:
		return GL_LUMINANCE;

	case GL_RGB16F:
	case GL_RGB32F:
	case GL_SRGB:
	case GL_SRGB_ALPHA:
	case GL_SRGB8:
	case GL_SRGB8_ALPHA8:
		return GL_RGB;

	case GL_RGBA16F:
	case GL_RGBA32F:
		return GL_RGBA;

	default:
		break;
	}
	return ifmt;
}

static int gltype_from_ifmt(unsigned int ifmt)
{
	switch(ifmt) {
	case GL_RGB16F:
	case GL_RGBA16F:
    case GL_LUMINANCE16F_ARB:
#ifdef GL_ES_VERSION_2_0
		return GL_HALF_FLOAT_OES;
#endif
	case GL_RGB32F:
	case GL_RGBA32F:
    case GL_LUMINANCE32F_ARB:
		return GL_FLOAT;

	default:
		break;
	}
	return GL_UNSIGNED_BYTE;
}

static int glifmt_from_imgfmt(Image::Format fmt)
{
	switch(fmt) {
	case Image::FMT_GREY:
		return GL_LUMINANCE;
	case Image::FMT_GREY_FLOAT:
        return GL_LUMINANCE16F_ARB;
	case Image::FMT_RGB:
		return GL_SRGB;
	case Image::FMT_RGB_FLOAT:
		return GL_RGB16F;
	case Image::FMT_RGBA:
		return GL_RGBA;
	case Image::FMT_RGBA_FLOAT:
		return GL_RGBA16F;
	default:
		break;
	}
	return 0;
}

// ---- TextureSet ----
static void destroy_texture(Texture *tex)
{
	delete tex;
}

TextureSet::TextureSet()
	: DataSet<Texture*>(load_texture, destroy_texture)
{
}

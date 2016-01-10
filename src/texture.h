#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "dataset.h"
#include "opengl.h"

namespace goatgfx {

class Image;

class Texture {
protected:
	unsigned int id;
	unsigned int target;
	unsigned int texfmt;
	int sz[3];

	Texture(const Texture &tex) {}
	Texture &operator =(const Texture &tex) { return *this; }

public:
	Texture();
	virtual ~Texture();

	void set_wrapping(unsigned int wrap);
	void set_filtering(unsigned int filt);
	void set_filtering(unsigned int min_filt, unsigned int mag_filt);

	unsigned int get_format() const;

	virtual int get_size(int dim) const;

	virtual void create(int xsz, int ysz, unsigned int ifmt = GL_RGBA) = 0;
	virtual void set_image(const Image &img, int idx = 0) = 0;

	virtual bool load(const char *fname) = 0;
	virtual bool save(const char *fname) const = 0;

	virtual unsigned int get_id() const;
	virtual void set_id(unsigned int id);

	virtual void bind(int tex_unit = 0) const;
};

class Texture2D : public Texture {
public:
	Texture2D();

	virtual void create(int xsz, int ysz, unsigned int ifmt = GL_RGBA);
	virtual void set_image(const Image &img, int idx = 0);

	virtual bool load(const char *fname);
	virtual bool save(const char *fname) const;
};

class TextureCube : public Texture {
private:
	bool load_multi(const Image &img, const float *xoffsets, const float *yoffsets, float sz,
		unsigned int rotmask = 0);

public:
	TextureCube();

	virtual void create(int xsz, int ysz, unsigned int ifmt = GL_RGBA);
	virtual void set_image(const Image &img, int idx = 0);

	virtual bool load(const char *fname);
	virtual bool save(const char *fname) const;
};

void set_texture(Texture *tex, int tunit = 0);

/** loads a texture autodetecting whether it's a 2D texture or
 * cubemap and creating the correct Texture subclass instance.
 */
Texture *load_texture(const char *fname);

class TextureSet : public DataSet<Texture*> {
public:
	TextureSet();
};

}	// namespace goatgfx

#endif	// TEXTURE_H_

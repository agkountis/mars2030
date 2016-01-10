#ifndef IMAGE_H_
#define IMAGE_H_

namespace goatgfx {

class Image {
public:
	enum Format {
		FMT_GREY,
		FMT_RGB,
		FMT_RGBA,
		FMT_GREY_FLOAT,
		FMT_RGB_FLOAT,
		FMT_RGBA_FLOAT
	};

private:
	Format fmt;
	int width, height;
	void *pixels;

public:
	Image();
	~Image();

	int get_width() const;
	int get_height() const;

	Format get_format() const;

	bool create(int x, int y, Format fmt = FMT_RGBA);
	bool set_pixels(int xsz, int ysz, void *pixels, Format fmt = FMT_RGBA);
	bool set_pixels(int xsz, int ysz, void *pixels, int scan_width, Format fmt = FMT_RGBA);
	bool set_pixels(int xsz, int ysz, void *pixels, int x, int y, int scan_width = -1, Format fmt = FMT_RGBA);
	void *get_pixels() const;

	void flip_horizontal();
	void flip_vertical();
	void rotate_180();

	bool load(const char *fname);
	bool save(const char *fname) const;
};

}	// namespace goatgfx

#endif	// IMAGE_H_

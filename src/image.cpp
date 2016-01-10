#include <string.h>

#ifndef _MSC_VER
#include <alloca.h>
#else
#include <malloc.h>
#endif

#include "imago2.h"
#include "image.h"
#include "logger.h"

using namespace goatgfx;

static int pixel_elements(Image::Format fmt);
static int elem_size(Image::Format fmt);
static int pixel_size(Image::Format fmt);

Image::Image()
{
	fmt = FMT_RGBA;
	width = height = 0;
	pixels = 0;
}

Image::~Image()
{
	delete [] (char*)pixels;
}

int Image::get_width() const
{
	return width;
}

int Image::get_height() const
{
	return height;
}

Image::Format Image::get_format() const
{
	return fmt;
}

bool Image::create(int x, int y, Format fmt)
{
	width = x;
	height = y;
	this->fmt = fmt;

	try {
		pixels = new char[x * y * pixel_size(fmt)];
	}
	catch(...) {
		return false;
	}
	return true;
}

bool Image::set_pixels(int xsz, int ysz, void *pixels, Format fmt)
{
	if(!create(xsz, ysz, fmt)) {
		return false;
	}
	memcpy(this->pixels, pixels, xsz * ysz * pixel_size(fmt));
	return true;
}

bool Image::set_pixels(int xsz, int ysz, void *pixels, int scan_width, Format fmt)
{
	return set_pixels(xsz, ysz, pixels, 0, 0, scan_width, fmt);
}

bool Image::set_pixels(int xsz, int ysz, void *pixels, int x, int y, int scan_width, Format fmt)
{
	if(scan_width <= 0) {
		scan_width = xsz;
	}

	if(!create(xsz, ysz, fmt)) {
		return false;
	}

	int pixsz = pixel_size(fmt);

	unsigned char *dest = (unsigned char*)this->pixels;
	unsigned char *src = (unsigned char*)pixels + (y * scan_width + x) * pixsz;
	for(int i=0; i<ysz; i++) {
		memcpy(dest, src, xsz * pixsz);
		dest += xsz * pixsz;
		src += scan_width * pixsz;
	}
	return true;
}

void *Image::get_pixels() const
{
	return pixels;
}

void Image::flip_horizontal()
{
	int pixsz = pixel_size(fmt);

	unsigned char *tmppix = (unsigned char*)alloca(pixsz);

	unsigned char *scan = (unsigned char*)pixels;
	for(int i=0; i<height; i++) {
		unsigned char *dest = scan;
		unsigned char *src = scan + (width - 1) * pixsz;

		while(src > dest) {
			memcpy(tmppix, src, pixsz);
			memcpy(src, dest, pixsz);
			memcpy(dest, tmppix, pixsz);
			dest += pixsz;
			src -= pixsz;
		}

		scan += width * pixsz;
	}
}

void Image::flip_vertical()
{
	int pixsz = pixel_size(fmt);

	unsigned char *tmpscan = (unsigned char*)alloca(width * pixsz);

	unsigned char *dest = (unsigned char*)pixels;
	unsigned char *src = (unsigned char*)pixels + (height - 1) * width * pixsz;

	while(src > dest) {
		memcpy(tmpscan, src, width * pixsz);
		memcpy(src, dest, width * pixsz);
		memcpy(dest, tmpscan, width * pixsz);
		dest += width * pixsz;
		src -= width * pixsz;
	}
}

void Image::rotate_180()
{
	flip_vertical();
	flip_horizontal();
}

bool Image::load(const char *fname)
{
	struct img_pixmap pixmap;

	img_init(&pixmap);
	if(img_load(&pixmap, fname) == -1) {
		return false;
	}

	Format fmt;
	switch(pixmap.fmt) {
	case IMG_FMT_GREY8:
		fmt = FMT_GREY;
		break;
	case IMG_FMT_RGB24:
		fmt = FMT_RGB;
		break;
	case IMG_FMT_RGBA32:
		fmt = FMT_RGBA;
		break;
	case IMG_FMT_GREYF:
		fmt = FMT_GREY_FLOAT;
		break;
	case IMG_FMT_RGBF:
		fmt = FMT_RGB_FLOAT;
		break;
	case IMG_FMT_RGBAF:
		fmt = FMT_RGBA_FLOAT;
		break;
	default:
		img_destroy(&pixmap);
		return false;
	}

	if(!set_pixels(pixmap.width, pixmap.height, pixmap.pixels, fmt)) {
		img_destroy(&pixmap);
		return false;
	}
	img_destroy(&pixmap);
	return true;
}

bool Image::save(const char *fname) const
{
	struct img_pixmap pixmap;

	img_init(&pixmap);

	switch(fmt) {
	case FMT_GREY:
		pixmap.fmt = IMG_FMT_GREY8;
		break;
	case FMT_GREY_FLOAT:
		pixmap.fmt = IMG_FMT_GREYF;
		break;
	case FMT_RGB:
		pixmap.fmt = IMG_FMT_RGB24;
		break;
	case FMT_RGB_FLOAT:
		pixmap.fmt = IMG_FMT_RGBF;
		break;
	case FMT_RGBA:
		pixmap.fmt = IMG_FMT_RGBA32;
		break;
	case FMT_RGBA_FLOAT:
		pixmap.fmt = IMG_FMT_RGBAF;
		break;
	default:
		return false;
	}

	pixmap.width = width;
	pixmap.height = height;
	pixmap.pixels = pixels;
	pixmap.pixelsz = pixel_size(fmt);

	if(img_save(&pixmap, fname) == -1) {
		return false;
	}
	return true;
}

static int pixel_elements(Image::Format fmt)
{
	switch(fmt) {
	case Image::FMT_GREY:
	case Image::FMT_GREY_FLOAT:
		return 1;

	case Image::FMT_RGB:
	case Image::FMT_RGB_FLOAT:
		return 3;

	case Image::FMT_RGBA:
	case Image::FMT_RGBA_FLOAT:
		return 4;

	default:
		break;
	}
	return 0;
}

static int elem_size(Image::Format fmt)
{
	switch(fmt) {
	case Image::FMT_GREY:
	case Image::FMT_RGB:
	case Image::FMT_RGBA:
		return 1;

	case Image::FMT_GREY_FLOAT:
	case Image::FMT_RGB_FLOAT:
	case Image::FMT_RGBA_FLOAT:
		return sizeof(float);

	default:
		break;
	}
	return 0;
}

static int pixel_size(Image::Format fmt)
{
	return elem_size(fmt) * pixel_elements(fmt);
}

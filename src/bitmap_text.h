#ifndef BITMAP_TEXT_H_
#define BITMAP_TEXT_H_
#include <iostream>
#include <string>
#include "drawtext.h"
#include "xform_node.h"
#include "shader.h"
#include "unistate.h"

enum TextAlignment {
	TXT_LEFT_ALIGNMENT, TXT_CENTER_ALIGNMENT, TXT_RIGHT_ALIGNEMENT
};

class BitmapText:public XFormNode
{
private:
	std::string text;
	dtx_font *font;

	float size;
	float bmp_string_width;
	float bmp_string_height;


	/*Encoding ranges*/
	int start_range;
	int end_range;

	/*Shader attributes*/
	int v_attr;
	int tex_attr;

	Vector4 font_color;

	ShaderProg *sdr;

	TextAlignment alignment;

	void align_left();
	void align_right();
	void aling_center();

public:
	BitmapText();

	void set_text(std::string text);
	void set_font(dtx_font *font);
	void set_font_size(float size);
	void set_font_color(const Vector4 &color);
	void set_encoding_range(int start_range, int end_range);
	void set_shader_attr_indices(int v_attr, int tex_attr);
	void set_shader_program(ShaderProg *sdr);
	void set_text_alignment(TextAlignment alignment);

	dtx_font *get_font() const;
	float get_font_size() const;
	const Vector4 &get_font_color() const;
	float get_bitmap_string_width() const;
	float get_bitmap_string_height() const;
	int get_encoding_start_range() const;
	int get_encoding_end_range() const;
	int get_vertex_attribute_idx() const;
	int get_tex_coord_attribute_idx() const;

	void init();
	void render(long time = 0);

	/*Use this as you would use a printf call. 
	 *Don't use the render function if you are going to use this one.
	 *The time argument is there in case the text has animation. Set to 0 if
	 *it does not.*/
	void bitmap_printf(long time , const char *fmt_txt, ...);
};

#endif

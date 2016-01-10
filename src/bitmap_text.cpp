#include "bitmap_text.h"

BitmapText::BitmapText()
{
	font = NULL;
	size = 1.0;

	start_range = 0;
	end_range = 256;

	v_attr = -1;
	tex_attr = -1;

	alignment = TXT_RIGHT_ALIGNEMENT;

	font_color = Vector4(0, 0, 0, 1);
	
	sdr = NULL;
}

void BitmapText::set_text(std::string text)
{
	this->text = text;
}

void BitmapText::set_font(dtx_font *font)
{
	this->font = font;
}

void BitmapText::set_font_size(float size)
{
	this->size = size;
}

void BitmapText::set_font_color(const Vector4 &color)
{
	font_color = color;
}

void BitmapText::set_encoding_range(int start_range, int end_range)
{
	this->start_range = start_range;
	this->end_range = end_range;
}

void BitmapText::set_shader_attr_indices(int v_attr, int tex_attr)
{
	this->v_attr = v_attr;
	this->tex_attr = tex_attr;
}

void BitmapText::set_shader_program(ShaderProg*sdr)
{
	this->sdr = sdr;
}

void BitmapText::set_text_alignment(TextAlignment alignment)
{
	this->alignment = alignment;
}

dtx_font* BitmapText::get_font() const
{
	return font;
}

float BitmapText::get_font_size() const
{
	return size;
}

const Vector4& BitmapText::get_font_color() const
{
	return font_color;
}

float BitmapText::get_bitmap_string_width() const
{
	return bmp_string_width;
}

float BitmapText::get_bitmap_string_height() const
{
	return bmp_string_height;
}

int BitmapText::get_encoding_start_range() const
{
	return start_range;
}

int BitmapText::get_encoding_end_range() const
{
	return end_range;
}

int BitmapText::get_vertex_attribute_idx() const
{
	return v_attr;
}

int BitmapText::get_tex_coord_attribute_idx() const
{
	return tex_attr;
}

void BitmapText::init()
{
	if (!font){
		std::cerr << "No font set! Cannot initialize the Bitmap text!" << std::endl;
		return;
	}

	dtx_prepare_range(font, size, start_range, end_range);
	dtx_vertex_attribs(v_attr, tex_attr);
}

void BitmapText::render(long time)
{
	Matrix4x4 offset_mat;
	Vector3 pos;
	Vector3 scale;

	bmp_string_width = dtx_string_width(text.c_str()) * get_scaling().x;
	bmp_string_height = dtx_string_height(text.c_str()) * get_scaling().y;

	pos = get_position();
	scale = get_scaling();

	dtx_use_font(font, size);

	if (sdr && (v_attr >= 0) && (tex_attr >= 0)){
		switch (alignment){
		case TXT_RIGHT_ALIGNEMENT:
			set_matrix(Matrix4x4());
			break;
		case TXT_CENTER_ALIGNMENT:
			offset_mat.reset_identity();
			offset_mat.translate(Vector3(pos.x - bmp_string_width / 2.0, 0.0, 0.0));
			set_matrix(offset_mat);
			break;
		case TXT_LEFT_ALIGNMENT:
			offset_mat.reset_identity();
			offset_mat.translate(Vector3(pos.x - bmp_string_width, 0.0, 0.0));
			set_matrix(offset_mat);
			break;
		}

		set_unistate("st_font_color", font_color);
		XFormNode::render(RENDER_OBJ, time);

		set_world_matrix(get_matrix());
		sdr->bind();
		dtx_string(text.c_str());
	}
	else{
		glUseProgram(0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glPushMatrix();

		switch (alignment){
		case TXT_RIGHT_ALIGNEMENT:
			glTranslatef(pos.x, pos.y, pos.z);
			glScalef(scale.x, scale.y, scale.z);
			break;
		case TXT_CENTER_ALIGNMENT:
			offset_mat.reset_identity();
			offset_mat.translate(Vector3(pos.x - bmp_string_width / 2.0, 0.0, 0.0));
			glTranslatef(pos.x, pos.y, pos.z);
			glScalef(scale.x, scale.y, scale.z);
			glMultMatrixf(offset_mat.transposed()[0]);
			break;
		case TXT_LEFT_ALIGNMENT:
			offset_mat.reset_identity();
			offset_mat.translate(Vector3(pos.x - bmp_string_width, 0.0, 0.0));
			glTranslatef(pos.x, pos.y, pos.z);
			glScalef(scale.x, scale.y, scale.z);
			glMultMatrixf(offset_mat.transposed()[0]);
			break;
		}
		dtx_string(text.c_str());
		glPopMatrix();
	}
}

void BitmapText::bitmap_printf(long time, const char *fmt_txt, ...)
{
	Matrix4x4 offset_mat;
	Vector3 pos;
	Vector3 scale;

	bmp_string_width = dtx_string_width(text.c_str()) * get_scaling().x;
	bmp_string_height = dtx_string_height(text.c_str()) * get_scaling().y;

	pos = get_position();
	scale = get_scaling();

	dtx_use_font(font, size);


	va_list ap;
	int buf_size;
	char *buf, tmp;

	if (!font) {
		return;
	}

	va_start(ap, fmt_txt);
	buf_size = vsnprintf(&tmp, 0, fmt_txt, ap);
	va_end(ap);

	if (buf_size == -1) {
		buf_size = 512;
	}

	buf = (char*)alloca(buf_size + 1);
	va_start(ap, fmt_txt);
	vsnprintf(buf, buf_size + 1, fmt_txt, ap);
	va_end(ap);

	if (sdr && (v_attr >= 0) && (tex_attr >= 0)){
		switch (alignment){
		case TXT_RIGHT_ALIGNEMENT:
			set_matrix(Matrix4x4());
			break;
		case TXT_CENTER_ALIGNMENT:
			offset_mat.reset_identity();
			offset_mat.translate(Vector3(pos.x - bmp_string_width / 2.0, 0.0, 0.0));
			set_matrix(offset_mat);
			break;
		case TXT_LEFT_ALIGNMENT:
			offset_mat.reset_identity();
			offset_mat.translate(Vector3(pos.x - bmp_string_width, 0.0, 0.0));
			set_matrix(offset_mat);
			break;
		}

		XFormNode::render(RENDER_OBJ, time);

		set_world_matrix(get_matrix());
		sdr->bind();
		dtx_string(buf);
	}
	else{
		glUseProgram(0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glPushMatrix();

		switch (alignment){
		case TXT_RIGHT_ALIGNEMENT:
			glTranslatef(pos.x, pos.y, pos.z);
			glScalef(scale.x, scale.y, scale.z);
			break;
		case TXT_CENTER_ALIGNMENT:
			offset_mat.reset_identity();
			offset_mat.translate(Vector3(pos.x - bmp_string_width / 2.0, 0.0, 0.0));
			glTranslatef(pos.x, pos.y, pos.z);
			glScalef(scale.x, scale.y, scale.z);
			glMultMatrixf(offset_mat.transposed()[0]);
			break;
		case TXT_LEFT_ALIGNMENT:
			offset_mat.reset_identity();
			offset_mat.translate(Vector3(pos.x - bmp_string_width, 0.0, 0.0));
			glTranslatef(pos.x, pos.y, pos.z);
			glScalef(scale.x, scale.y, scale.z);
			glMultMatrixf(offset_mat.transposed()[0]);
			break;
		}
		dtx_string(buf);
		glPopMatrix();
	}
}

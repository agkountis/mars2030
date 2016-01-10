#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "opengl.h"
#include "shader.h"
#include "logger.h"
#include "unistate.h"

#ifdef _MSC_VER
#include <malloc.h>
#else
#include <alloca.h>
#endif

#ifdef __GLEW_H__
#define HAVE_GEOMETRY_SHADER
#define HAVE_TESSELATION_SHADER
#endif

static const char *strtype(unsigned int type);

ShaderProg *ShaderProg::current;

Shader::Shader()
{
	sdr = type = 0;
	name = 0;
}

Shader::~Shader()
{
	destroy();
}

unsigned int Shader::get_id() const
{
	return sdr;
}

void Shader::set_name(const char *name)
{
	delete [] this->name;
	this->name = new char[strlen(name) + 1];
	strcpy(this->name, name);
}

const char *Shader::get_name() const
{
	return name;
}

bool Shader::create(const char *src, unsigned int type)
{
#if !GL_ES_VERSION_2_0
	const char *src_arr[] = {src};
#else
	const char *src_arr[] = { "precision mediump float; ", src };
#endif

	if(!sdr) {
		sdr = glCreateShader(type);
	}

	info_log("compiling shader: %s... ", name ? name : "");

	glShaderSource(sdr, sizeof src_arr / sizeof *src_arr, src_arr, 0);
	glCompileShader(sdr);

	int status;
	glGetShaderiv(sdr, GL_COMPILE_STATUS, &status);

	info_log(status ? "success\n" : "failed\n");

	int info_len;
	glGetShaderiv(sdr, GL_INFO_LOG_LENGTH, &info_len);
	if(info_len > 1) {
		char *buf = (char*)alloca(info_len);
		glGetShaderInfoLog(sdr, info_len, 0, buf);
		buf[info_len - 1] = 0;

		if(status) {
			info_log("%s\n", buf);
		} else {
			error_log("%s\n", buf);
		}
	}

	return status == GL_TRUE;
}

void Shader::destroy()
{
	if(sdr) {
		glDeleteShader(sdr);
	}
	sdr = type = 0;

	delete [] name;
	name = 0;
}

bool Shader::load(const char *fname, unsigned int type)
{
	FILE *fp;

	if(!(fp = fopen(datafile_path(fname).c_str(), "rb"))) {
		error_log("failed to load %s shader: %s: %s\n", strtype(type), fname, strerror(errno));
		return false;
	}

	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	rewind(fp);

	char *src = (char*)alloca(sz + 1);
	if(fread(src, 1, sz, fp) < (size_t)sz) {
		error_log("failed to load %s shader: %s: %s\n", strtype(type), fname, strerror(errno));
		fclose(fp);
		return false;
	}
	src[sz] = 0;
	fclose(fp);

	set_name(fname);
	return create(src, type);
}

// ---- shader program ----
ShaderProg::ShaderProg()
{
	prog = 0;
	must_link = true;
}

ShaderProg::~ShaderProg()
{
	destroy();
}

unsigned int ShaderProg::get_id() const
{
	return prog;
}

bool ShaderProg::create(const char *src, unsigned int type, ...)
{
	va_list ap;

	va_start(ap, type);
	bool res = create(src, type, ap);
	va_end(ap);

	return res;
}

bool ShaderProg::create(const char *src, unsigned int type, va_list ap)
{
	destroy();
	prog = glCreateProgram();

	while(src) {
		Shader *sdr = new Shader;
		if(!sdr->create(src, type)) {
			va_end(ap);
			return false;
		}
		add_shader(sdr);
		src = va_arg(ap, const char*);
		type = va_arg(ap, unsigned int);
	}

	return link();
}

bool ShaderProg::create(const char *vsrc, const char *psrc)
{
	return create(VSDR(vsrc), PSDR(psrc), 0);
}

bool ShaderProg::create(Shader *sdr, ...)
{
	va_list ap;

	va_start(ap, sdr);
	bool res = create(sdr, ap);
	va_end(ap);

	return res;
}

bool ShaderProg::create(Shader *sdr, va_list ap)
{
	destroy();
	prog = glCreateProgram();

	while(sdr) {
		add_shader(sdr);
		sdr = va_arg(ap, Shader*);
	}
	return link();
}

bool ShaderProg::create(Shader *vsdr, Shader *psdr)
{
	return create(vsdr, psdr, 0);
}

void ShaderProg::destroy()
{
	if(prog) {
		glDeleteProgram(prog);
	}
	prog = 0;

	shaders.clear();
	// don't actually destroy the shaders, let the ShaderSet own them
}

bool ShaderProg::load(const char *fname, unsigned int type, ...)
{
	va_list ap;
	va_start(ap, type);
	bool res = load(fname, type, ap);
	va_end(ap);

	return res;
}

bool ShaderProg::load(const char *fname, unsigned int type, va_list ap)
{
	destroy();
	prog = glCreateProgram();

	while(fname) {
		Shader *sdr = new Shader;
		if(!sdr->load(fname, type)) {
			delete sdr;
			return false;
		}
		add_shader(sdr);

		if((fname = va_arg(ap, const char*))) {
			type = va_arg(ap, unsigned int);
		}
	}

	return link();
}

bool ShaderProg::load(const char *vfname, const char *pfname)
{
	return load(VSDR(vfname), PSDR(pfname), 0);
}

void ShaderProg::add_shader(Shader *sdr)
{
	glAttachShader(prog, sdr->get_id());
}

bool ShaderProg::link() const
{
	CHECKGLERR;
	info_log("linking program ... ");
	glLinkProgram(prog);

	int status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);

	info_log(status ? "success\n" : "failed\n");

	int info_len;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &info_len);
	if(info_len > 1) {
		char *buf = (char*)alloca(info_len);
		glGetProgramInfoLog(prog, info_len, 0, buf);
		buf[info_len - 1] = 0;

		if(status) {
			info_log("%s\n", buf);
		} else {
			error_log("%s\n", buf);
		}
	}

	if(status) {
		must_link = false;
		cache_state_uniforms();
		return true;
	}
	return false;
}

void ShaderProg::bind() const
{
	CHECKGLERR;
	if(must_link) {
		if(!link()) {
			return;
		}
	}
	CHECKGLERR;
	glUseProgram(prog);
	ShaderProg::current = (ShaderProg*)this;

	setup_state_uniforms();
}


int ShaderProg::get_attrib_location(const char *name) const
{
	glUseProgram(prog);
	return glGetAttribLocation(prog, name);
}

void ShaderProg::set_attrib_location(const char *name, int loc) const
{
	glBindAttribLocation(prog, loc, name);
	must_link = true;
}

int ShaderProg::get_uniform_location(const char *name) const
{
	glUseProgram(prog);
	return glGetUniformLocation(prog, name);
}

bool ShaderProg::set_uniform(int loc, int val) const
{
	glUseProgram(prog);
	if(loc >= 0) {
		glUniform1i(loc, val);
		return true;
	}
	return false;
}

bool ShaderProg::set_uniform(int loc, float val) const
{
	glUseProgram(prog);
	if(loc >= 0) {
		glUniform1f(loc, val);
		return true;
	}
	return false;
}

bool ShaderProg::set_uniform(int loc, const Vector2 &v) const
{
	glUseProgram(prog);
	if(loc >= 0) {
		glUniform2f(loc, v.x, v.y);
		return true;
	}
	return false;
}

bool ShaderProg::set_uniform(int loc, const Vector3 &v) const
{
	glUseProgram(prog);
	if(loc >= 0) {
		glUniform3f(loc, v.x, v.y, v.z);
		return true;
	}
	return false;
}

bool ShaderProg::set_uniform(int loc, const Vector4 &v) const
{
	glUseProgram(prog);
	if(loc >= 0) {
		glUniform4f(loc, v.x, v.y, v.z, v.w);
		return true;
	}
	return false;
}

bool ShaderProg::set_uniform(int loc, const Matrix3x3 &m) const
{
	glUseProgram(prog);
	if(loc >= 0) {
		glUniformMatrix3fv(loc, 1, GL_TRUE, m[0]);
		return true;
	}
	return false;
}

bool ShaderProg::set_uniform(int loc, const Matrix4x4 &m) const
{
	glUseProgram(prog);
	if(loc >= 0) {
		glUniformMatrix4fv(loc, 1, GL_TRUE, m[0]);
		return true;
	}
	return false;
}


bool ShaderProg::set_uniform(const char *name, int val) const
{
	return set_uniform(get_uniform_location(name), val);
}

bool ShaderProg::set_uniform(const char *name, float val) const
{
	return set_uniform(get_uniform_location(name), val);
}

bool ShaderProg::set_uniform(const char *name, const Vector2 &v) const
{
	return set_uniform(get_uniform_location(name), v);
}

bool ShaderProg::set_uniform(const char *name, const Vector3 &v) const
{
	return set_uniform(get_uniform_location(name), v);
}

bool ShaderProg::set_uniform(const char *name, const Vector4 &v) const
{
	return set_uniform(get_uniform_location(name), v);
}

bool ShaderProg::set_uniform(const char *name, const Matrix3x3 &m) const
{
	return set_uniform(get_uniform_location(name), m);
}

bool ShaderProg::set_uniform(const char *name, const Matrix4x4 &m) const
{
	return set_uniform(get_uniform_location(name), m);
}

static StType unist_type(GLenum type)
{
	switch(type) {
	case GL_FLOAT:
		return ST_FLOAT;
	case GL_FLOAT_VEC2:
		return ST_FLOAT2;
	case GL_FLOAT_VEC3:
		return ST_FLOAT3;
	case GL_FLOAT_VEC4:
		return ST_FLOAT4;
	case GL_INT:
	case GL_SAMPLER_2D:
	case GL_SAMPLER_CUBE:
#if !GL_ES_VERSION_2_0
	case GL_SAMPLER_1D:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_1D_SHADOW:
	case GL_SAMPLER_2D_SHADOW:
#endif
		return ST_INT;
	case GL_INT_VEC2:
		return ST_INT2;
	case GL_INT_VEC3:
		return ST_INT3;
	case GL_INT_VEC4:
		return ST_INT4;
	case GL_FLOAT_MAT3:
		return ST_MATRIX3;
	case GL_FLOAT_MAT4:
		return ST_MATRIX4;
	default:
		break;
	}
	return ST_UNKNOWN;
}

void ShaderProg::cache_state_uniforms() const
{
	if(!glIsProgram(prog)) {
		return;
	}

	int num_uni;
	glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &num_uni);

	char name[256];
	for(int i=0; i<num_uni; i++) {
		GLint sz;
		GLenum type;
		glGetActiveUniform(prog, i, sizeof name - 1, 0, &sz, &type, name);

		if(strstr(name, "st_") == name) {
			StateLocCache s;
			s.sidx = add_unistate(name, unist_type(type));
			s.loc = glGetUniformLocation(prog, name);
            s.name = name;
			stloc_cache.push_back(s);
		}
	}
}

void ShaderProg::setup_state_uniforms() const
{
	for(size_t i=0; i<stloc_cache.size(); i++) {
		setup_unistate(stloc_cache[i].sidx, this, stloc_cache[i].loc);
		CHECKGLERR;
	}
}

// ---- ShaderSet ----
static Shader *load_shader(const char *fname, unsigned int type)
{
	Shader *sdr = new Shader;
	if(!sdr->load(fname, type)) {
		delete sdr;
		return 0;
	}
	return sdr;
}

static Shader *load_vertex_shader(const char *fname)
{
	return load_shader(fname, GL_VERTEX_SHADER);
}

static Shader *load_pixel_shader(const char *fname)
{
	return load_shader(fname, GL_FRAGMENT_SHADER);
}

#ifdef HAVE_GEOMETRY_SHADER
static Shader *load_geom_shader(const char *fname)
{
	return load_shader(fname, GL_GEOMETRY_SHADER);
}
#endif

#ifdef HAVE_TESSELATION_SHADER
static Shader *load_tc_shader(const char *fname)
{
	return load_shader(fname, GL_TESS_CONTROL_SHADER);
}

static Shader *load_te_shader(const char *fname)
{
	return load_shader(fname, GL_TESS_EVALUATION_SHADER);
}
#endif

static void destroy_shader(Shader *sdr)
{
	delete sdr;
}

ShaderSet::ShaderSet(unsigned int type)
	: DataSet<Shader*>(0, destroy_shader)
{
	switch(type) {
	case GL_VERTEX_SHADER:
		load = load_vertex_shader;
		break;

	case GL_FRAGMENT_SHADER:
		load = load_pixel_shader;
		break;

#ifdef HAVE_GEOMETRY_SHADER
	case GL_GEOMETRY_SHADER:
		load = load_geom_shader;
		break;
#endif

#ifdef HAVE_TESSELATION_SHADER
	case GL_TESS_CONTROL_SHADER:
		load = load_tc_shader;
		break;

	case GL_TESS_EVALUATION_SHADER:
		load = load_te_shader;
		break;
#endif

	default:
		error_log("ShaderSet constructed with invalid shader type!\n");
	}
}

static const char *strtype(unsigned int type)
{
	switch(type) {
	case GL_VERTEX_SHADER:
		return "vertex";
	case GL_FRAGMENT_SHADER:
		return "fragment";
#ifdef HAVE_GEOMETRY_SHADER
	case GL_GEOMETRY_SHADER:
		return "geometry";
#endif
#ifdef HAVE_TESSELATION_SHADER
	case GL_TESS_CONTROL_SHADER:
		return "tesselation control";
	case GL_TESS_EVALUATION_SHADER:
		return "tesselation evaluation";
#endif
	default:
		break;
	}
	return "<unknown>";
}

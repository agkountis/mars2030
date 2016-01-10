#include <map>
#include <vector>
#include "unistate.h"
#include "shader.h"
#include "logger.h"

struct StateItem {
	StType type;

	union {
		int ival[4];
		float fval[16];
	};
	int transpose;	// for matrices
};

static const char *typestr(StType type);
static int type_nelem(StType type);
static StType float_type(int elem);
static StType int_type(int elem);

std::vector<StateItem> state;
std::map<std::string, int> stateidx;


int add_unistate(const char *name, StType type)
{
	static const float ident3[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
	static const float ident4[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	if(stateidx.find(name) != stateidx.end()) {
		return stateidx[name];
	}

	StateItem sitem;
	memset(&sitem, 0, sizeof sitem);
	sitem.type = type;

	// initialize to a reasonable default value
	switch(type) {
	case ST_MATRIX3:
		memcpy(sitem.fval, ident3, sizeof ident3);
		break;

	case ST_MATRIX4:
		memcpy(sitem.fval, ident4, sizeof ident4);
		break;

	default:
		break;	// in all other cases leave it zero (see memset above)
	}

	int sidx = state.size();
	state.push_back(sitem);
	stateidx[name] = sidx;

	debug_log("adding uniform state [%d]: %s %s\n", sidx, typestr(sitem.type), name);

	return sidx;
}

int get_unistate_index(const char *name)
{
	std::map<std::string, int>::const_iterator it = stateidx.find(name);
	if(it != stateidx.end()) {
		return it->second;
	}
	return -1;
}

#define CHECK_INDEX(i)	\
	if(i < 0 || i >= (int)state.size()) return

#define CHECK_COUNT(count, type) \
	do { \
		int max_elem = type_nelem(type); \
		if(!(count) || (count) > max_elem) { \
			count = max_elem; \
		} \
	} while(0)

void set_unistate(int sidx, const int *val, int count)
{
	CHECK_INDEX(sidx);
	CHECK_COUNT(count, state[sidx].type);

	memcpy(state[sidx].ival, val, count * sizeof *state[sidx].ival);
}

void set_unistate(int sidx, const float *val, int count)
{
	CHECK_INDEX(sidx);
	CHECK_COUNT(count, state[sidx].type);

	memcpy(state[sidx].fval, val, count * sizeof *state[sidx].fval);
	state[sidx].transpose = 0;
}

void get_unistate(int sidx, int *val, int count)
{
	CHECK_INDEX(sidx);
	CHECK_COUNT(count, state[sidx].type);

	memcpy(val, state[sidx].ival, count * sizeof *val);
}

void get_unistate(int sidx, float *val, int count)
{
	CHECK_INDEX(sidx);
	CHECK_COUNT(count, state[sidx].type);

	memcpy(val, state[sidx].fval, count * sizeof *val);
}

void set_unistate(int sidx, int val)
{
	set_unistate(sidx, &val, 1);
}

void set_unistate(int sidx, float val)
{
	set_unistate(sidx, &val, 1);
}

void set_unistate(int sidx, const Vector2 &vec)
{
	set_unistate(sidx, &vec.x, 2);
}

void set_unistate(int sidx, const Vector3 &vec)
{
	set_unistate(sidx, &vec.x, 3);
}

void set_unistate(int sidx, const Vector4 &vec)
{
	set_unistate(sidx, &vec.x, 4);
}

void set_unistate(int sidx, const Matrix3x3 &mat)
{
	set_unistate(sidx, mat[0], 9);
	state[sidx].transpose = 1;
}

void set_unistate(int sidx, const Matrix4x4 &mat)
{
	set_unistate(sidx, mat[0], 16);
	state[sidx].transpose = 1;
}


int set_unistate(const char *name, int *val, int count)
{
	int sidx = get_unistate_index(name);
	if(sidx < 0) {
		StType type = int_type(count);
		if(type == ST_UNKNOWN) {
			error_log("invalid element count (%d) while setting previously unknown unistate item \"%s\"\n",
					count, name);
			return -1;
		}

		sidx = add_unistate(name, type);
	}
	set_unistate(sidx, val);
	return sidx;
}

int set_unistate(const char *name, float *val, int count)
{
	int sidx = get_unistate_index(name);
	if(sidx < 0) {
		StType type = float_type(count);
		if(type == ST_UNKNOWN) {
			error_log("invalid element count (%d) while setting previously unknown unistate item \"%s\"\n",
					count, name);
			return -1;
		}

		sidx = add_unistate(name, type);
	}
	set_unistate(sidx, val);
	return sidx;
}

int set_unistate(const char *name, int val)
{
	int sidx = get_unistate_index(name);
	if(sidx < 0) {
		sidx = add_unistate(name, ST_INT);
	}
	set_unistate(sidx, val);
	return sidx;
}

int set_unistate(const char *name, float val)
{
	int sidx = get_unistate_index(name);
	if(sidx < 0) {
		sidx = add_unistate(name, ST_FLOAT);
	}
	set_unistate(sidx, val);
	return sidx;
}

int set_unistate(const char *name, const Vector2 &vec)
{
	int sidx = get_unistate_index(name);
	if(sidx < 0) {
		sidx = add_unistate(name, ST_FLOAT2);
	}
	set_unistate(sidx, vec);
	return sidx;
}

int set_unistate(const char *name, const Vector3 &vec)
{
	int sidx = get_unistate_index(name);
	if(sidx < 0) {
		sidx = add_unistate(name, ST_FLOAT3);
	}
	set_unistate(sidx, vec);
	return sidx;
}

int set_unistate(const char *name, const Vector4 &vec)
{
	int sidx = get_unistate_index(name);
	if(sidx < 0) {
		sidx = add_unistate(name, ST_FLOAT4);
	}
	set_unistate(sidx, vec);
	return sidx;
}

int set_unistate(const char *name, const Matrix3x3 &mat)
{
	int sidx = get_unistate_index(name);
	if(sidx < 0) {
		sidx = add_unistate(name, ST_MATRIX3);
	}
	set_unistate(sidx, mat);
	return sidx;
}

int set_unistate(const char *name, const Matrix4x4 &mat)
{
	int sidx = get_unistate_index(name);
	if(sidx < 0) {
		sidx = add_unistate(name, ST_MATRIX4);
	}
	set_unistate(sidx, mat);
	return sidx;
}


int get_unistate_int(int sidx)
{
	int val = 0;
	get_unistate(sidx, &val, 1);
	return val;
}

float get_unistate_float(int sidx)
{
	float val = 0.0f;
	get_unistate(sidx, &val, 1);
	return val;
}

Vector2 get_unistate_vec2(int sidx)
{
	float val[2] = {0.0f, 0.0f};
	get_unistate(sidx, val, 2);
	return Vector2(val[0], val[1]);
}

Vector3 get_unistate_vec3(int sidx)
{
	float val[3] = {0.0f, 0.0f, 0.0f};
	get_unistate(sidx, val, 3);
	return Vector3(val[0], val[1], val[2]);
}

Vector4 get_unistate_vec4(int sidx)
{
	float val[4] = {0.0f, 0.0f, 0.0f};
	get_unistate(sidx, val, 4);
	return Vector4(val[0], val[1], val[2], val[3]);
}

Matrix3x3 get_unistate_mat3(int sidx)
{
	Matrix3x3 res;
	get_unistate(sidx, res.m[0], 9);
	return res;
}

Matrix4x4 get_unistate_mat4(int sidx)
{
	Matrix4x4 res;
	get_unistate(sidx, res.m[0], 16);
	return res;
}


int get_unistate_int(const char *name)
{
	int sidx = get_unistate_index(name);
	if(sidx == -1) {
		return 0;
	}
	return get_unistate_int(sidx);
}

float get_unistate_float(const char *name)
{
	int sidx = get_unistate_index(name);
	if(sidx == -1) {
		return 0.0f;
	}
	return get_unistate_float(sidx);
}

Vector2 get_unistate_vec2(const char *name)
{
	int sidx = get_unistate_index(name);
	if(sidx == -1) {
		return Vector2();
	}
	return get_unistate_vec2(sidx);
}

Vector3 get_unistate_vec3(const char *name)
{
	int sidx = get_unistate_index(name);
	if(sidx == -1) {
		return Vector3();
	}
	return get_unistate_vec3(sidx);
}

Vector4 get_unistate_vec4(const char *name)
{
	int sidx = get_unistate_index(name);
	if(sidx == -1) {
		return Vector4();
	}
	return get_unistate_vec4(sidx);
}

Matrix3x3 get_unistate_mat3(const char *name)
{
	int sidx = get_unistate_index(name);
	if(sidx == -1) {
		return Matrix3x3();
	}
	return get_unistate_mat3(sidx);
}

Matrix4x4 get_unistate_mat4(const char *name)
{
	int sidx = get_unistate_index(name);
	if(sidx == -1) {
		return Matrix4x4();
	}
	return get_unistate_mat4(sidx);
}


void setup_unistate(const ShaderProg *sdr)
{
	if(!sdr) {
		if(!(sdr = ShaderProg::current)) {
			return;
		}
	}

	sdr->setup_state_uniforms();
}

bool setup_unistate(int sidx, const ShaderProg *sdr, int loc)
{
	if(loc < 0 || sidx < 0 || sidx >= (int)state.size()) {
		return false;
	}

	CHECKGLERR;
	glUseProgram(sdr->get_id());
	CHECKGLERR;

	switch(state[sidx].type) {
	case ST_INT:
		glUniform1iv(loc, 1, state[sidx].ival);
		break;
	case ST_INT2:
		glUniform2iv(loc, 1, state[sidx].ival);
		break;
	case ST_INT3:
		glUniform3iv(loc, 1, state[sidx].ival);
		break;
	case ST_INT4:
		glUniform4iv(loc, 1, state[sidx].ival);
		break;

	case ST_FLOAT:
		glUniform1fv(loc, 1, state[sidx].fval);
		break;
	case ST_FLOAT2:
		glUniform2fv(loc, 1, state[sidx].fval);
		break;
	case ST_FLOAT3:
		glUniform3fv(loc, 1, state[sidx].fval);
		break;
	case ST_FLOAT4:
		glUniform4fv(loc, 1, state[sidx].fval);
		break;

	case ST_MATRIX3:
#ifdef GL_ES_VERSION_2_0
		{
			float tmat[9], *ptr = tmat;
			for(int i=0; i<3; i++) {
				for(int j=0; j<3; j++) {
					*ptr++ = state[sidx].fval[j * 3 + i];
				}
			}
			glUniformMatrix3fv(loc, 1, GL_FALSE, tmat);
		}
#else
		glUniformMatrix3fv(loc, 1, state[sidx].transpose, state[sidx].fval);
#endif
		break;

	case ST_MATRIX4:
#ifdef GL_ES_VERSION_2_0
		{
			float tmat[16], *ptr = tmat;
			for(int i=0; i<4; i++) {
				for(int j=0; j<4; j++) {
					*ptr++ = state[sidx].fval[j * 4 + i];
				}
			}
			glUniformMatrix4fv(loc, 1, GL_FALSE, tmat);
		}
#else
		glUniformMatrix4fv(loc, 1, state[sidx].transpose, state[sidx].fval);
#endif
		break;

	default:
		return false;
	}

	CHECKGLERR;
	return true;
}

bool setup_unistate(const char *name, const ShaderProg *sdr)
{
	int loc = sdr->get_uniform_location(name);
	if(loc == -1) {
		return false;
	}
	return setup_unistate(get_unistate_index(name), sdr, loc);
}

void set_world_matrix(const Matrix4x4 &mat)
{
	static int sidx = -1, sidx_transp, sidx_mat3, sidx_inv;

	if(sidx == -1) {
		sidx = add_unistate("st_world_matrix", ST_MATRIX4);
		sidx_mat3 = add_unistate("st_world_matrix3", ST_MATRIX3);
		sidx_transp = add_unistate("st_world_matrix_transpose", ST_MATRIX4);
		sidx_inv = add_unistate("st_world_matrix_inverse", ST_MATRIX4);
	}

	set_unistate(sidx, mat);
	set_unistate(sidx_mat3, Matrix3x3(mat));
	set_unistate(sidx_transp, mat[0]);	// by using the float* variant, we unset the transpose flag
	set_unistate(sidx_inv, mat.inverse());
}

void set_view_matrix(const Matrix4x4 &mat)
{
	static int sidx = -1, sidx_transp, sidx_mat3, sidx_inv;

	if(sidx == -1) {
		sidx = add_unistate("st_view_matrix", ST_MATRIX4);
		sidx_mat3 = add_unistate("st_view_matrix3", ST_MATRIX3);
		sidx_transp = add_unistate("st_view_matrix_transpose", ST_MATRIX4);
		sidx_inv = add_unistate("st_view_matrix_inverse", ST_MATRIX4);
	}

	set_unistate(sidx, mat);
	set_unistate(sidx_mat3, Matrix3x3(mat));
	set_unistate(sidx_transp, mat[0]);	// by using the float* variant, we unset the transpose flag
	set_unistate(sidx_inv, mat.inverse());
}

void set_projection_matrix(const Matrix4x4 &mat)
{
	static int sidx = -1;

	if(sidx == -1) {
		sidx = add_unistate("st_proj_matrix", ST_MATRIX4);
	}

	set_unistate(sidx, mat);
}

void set_texture_matrix(const Matrix4x4 &mat)
{
	static int sidx = -1;

	if(sidx == -1) {
		sidx = add_unistate("st_tex_matrix", ST_MATRIX4);
	}

	set_unistate(sidx, mat);
}

Matrix4x4 get_world_matrix()
{
	static int sidx = -1;

	if(sidx == -1) {
		if((sidx = get_unistate_index("st_world_matrix")) == -1) {
			return Matrix4x4();
		}
	}
	return get_unistate_mat4(sidx);
}

Matrix4x4 get_view_matrix()
{
	static int sidx = -1;

	if(sidx == -1) {
		if((sidx = get_unistate_index("st_view_matrix")) == -1) {
			return Matrix4x4();
		}
	}
	return get_unistate_mat4(sidx);
}

Matrix4x4 get_projection_matrix()
{
	static int sidx = -1;

	if(sidx == -1) {
		if((sidx = get_unistate_index("st_proj_matrix")) == -1) {
			return Matrix4x4();
		}
	}
	return get_unistate_mat4(sidx);
}

Matrix4x4 get_texture_matrix()
{
	static int sidx = -1;

	if(sidx == -1) {
		if((sidx = get_unistate_index("st_tex_matrix")) == -1) {
			return Matrix4x4();
		}
	}
	return get_unistate_mat4(sidx);
}

void setup_gl_matrices()
{
#ifdef USE_OLDGL
	Matrix4x4 modelview = get_world_matrix() * get_view_matrix();
	Matrix4x4 proj = get_projection_matrix();
	Matrix4x4 tex = get_texture_matrix();

	glMatrixMode(GL_TEXTURE);
	glLoadTransposeMatrixf(tex[0]);
	glMatrixMode(GL_PROJECTION);
	glLoadTransposeMatrixf(proj[0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadTransposeMatrixf(modelview[0]);
#endif
}

static const char *typestr(StType type)
{
	switch(type) {
	case ST_INT:
		return "int";
	case ST_INT2:
		return "ivec2";
	case ST_INT3:
		return "ivec3";
	case ST_INT4:
		return "ivec4";
	case ST_FLOAT:
		return "float";
	case ST_FLOAT2:
		return "vec2";
	case ST_FLOAT3:
		return "vec3";
	case ST_FLOAT4:
		return "vec4";
	case ST_MATRIX3:
		return "mat3";
	case ST_MATRIX4:
		return "mat4";

	default:
		break;
	}
	return "<unknown>";
}

static int type_nelem(StType type)
{
	switch(type) {
	case ST_INT:
	case ST_FLOAT:
		return 1;
	case ST_INT2:
	case ST_FLOAT2:
		return 2;
	case ST_INT3:
	case ST_FLOAT3:
		return 3;
	case ST_INT4:
	case ST_FLOAT4:
		return 4;
	case ST_MATRIX3:
		return 9;
	case ST_MATRIX4:
		return 16;

	default:
		break;
	}

	return 0;
}

static StType float_type(int elem)
{
	switch(elem) {
	case 1:
		return ST_FLOAT;
	case 2:
		return ST_FLOAT2;
	case 3:
		return ST_FLOAT3;
	case 4:
		return ST_FLOAT4;
	case 9:
		return ST_MATRIX3;
	case 16:
		return ST_MATRIX4;
	default:
		break;
	}
	return ST_UNKNOWN;
}

static StType int_type(int elem)
{
	switch(elem) {
	case 1:
		return ST_INT;
	case 2:
		return ST_INT2;
	case 3:
		return ST_INT3;
	case 4:
		return ST_INT4;
	default:
		break;
	}
	return ST_UNKNOWN;
}

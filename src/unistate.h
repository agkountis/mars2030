#ifndef UNISTATE_H_
#define UNISTATE_H_

#include "vmath/vmath.h"

class ShaderProg;

enum StType {
	ST_UNKNOWN,
	ST_INT,	ST_INT2, ST_INT3, ST_INT4,
	ST_FLOAT, ST_FLOAT2, ST_FLOAT3, ST_FLOAT4,
	ST_MATRIX3, ST_MATRIX4
};

int add_unistate(const char *name, StType type);
int get_unistate_index(const char *name);

/** set the uniform state identified by \param sidx by copying
 * a number of elements from \param val. If \param count is 0
 * then it's automatically set based on the type of this state item.
 * @{ */
void set_unistate(int sidx, const int *val, int count = 0);
void set_unistate(int sidx, const float *val, int count = 0);
/// @}

/** get the uniform state identified by \param sidx by copying
 * a number of elements into \param val. If \param count is 0
 * then it's automatically set based on the type of this state item.
 * @{ */
void get_unistate(int sidx, int *val, int count = 0);
void get_unistate(int sidx, float *val, int count = 0);
/// @}

/// convenience versions of set_unistate @{
void set_unistate(int sidx, int val);
void set_unistate(int sidx, float val);
void set_unistate(int sidx, const Vector2 &vec);
void set_unistate(int sidx, const Vector3 &vec);
void set_unistate(int sidx, const Vector4 &vec);
void set_unistate(int sidx, const Matrix3x3 &mat);
void set_unistate(int sidx, const Matrix4x4 &mat);
/// @}

/** convenience functions for setting the uniform state by name.
 * if the name cannot be found in the current set of uniform state
 * items, a new one is created with a type derived from the variant
 * of the function that was called (which might not be what you want).
 * The index of the state item is returned.
 * @{ */
int set_unistate(const char *name, int *val, int count = 0);
int set_unistate(const char *name, float *val, int count = 0);
int set_unistate(const char *name, int val);
int set_unistate(const char *name, float val);
int set_unistate(const char *name, const Vector2 &vec);
int set_unistate(const char *name, const Vector3 &vec);
int set_unistate(const char *name, const Vector4 &vec);
int set_unistate(const char *name, const Matrix3x3 &mat);
int set_unistate(const char *name, const Matrix4x4 &mat);
/// @}

/// convenience versions of get_unistate @{
int get_unistate_int(int sidx);
float get_unistate_float(int sidx);
Vector2 get_unistate_vec2(int sidx);
Vector3 get_unistate_vec3(int sidx);
Vector4 get_unistate_vec4(int sidx);
Matrix3x3 get_unistate_mat3(int sidx);
Matrix4x4 get_unistate_mat4(int sidx);
/// @}

/// convenience versions of get_unistate for getting the uniform state by name @{
int get_unistate_int(const char *name);
float get_unistate_float(const char *name);
Vector2 get_unistate_vec2(const char *name);
Vector3 get_unistate_vec3(const char *name);
Vector4 get_unistate_vec4(const char *name);
Matrix3x3 get_unistate_mat3(const char *name);
Matrix4x4 get_unistate_mat4(const char *name);
/// @}

/** Prepare for rendering by setting up all the state uniforms in the shader sdr.
 * If sdr is null, then use the "current" shader as per ShaderProg::current
 */
void setup_unistate(const ShaderProg *sdr = 0);

bool setup_unistate(int sidx, const ShaderProg *sdr, int loc);
bool setup_unistate(const char *name, const ShaderProg *sdr);

// special functions for setting the rendering pipeline matrices
void set_world_matrix(const Matrix4x4 &mat);
void set_view_matrix(const Matrix4x4 &mat);
void set_projection_matrix(const Matrix4x4 &mat);
void set_texture_matrix(const Matrix4x4 &mat);

Matrix4x4 get_world_matrix();
Matrix4x4 get_view_matrix();
Matrix4x4 get_projection_matrix();
Matrix4x4 get_texture_matrix();

void setup_gl_matrices();	// this shouldn't be needed in the final code

// TODO should do a matrix stack at some point ...

#endif	// UNISTATE_H_

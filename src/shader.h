#ifndef SHADER_H_
#define SHADER_H_

#include <vector>
#include "vmath/vmath.h"
#include "opengl.h"
#include "dataset.h"
#include <string>

class Shader {
private:
	unsigned int sdr;
	unsigned int type;
	char *name;

public:
	Shader();
	~Shader();

	unsigned int get_id() const;

	void set_name(const char *name);
	const char *get_name() const;

	bool create(const char *src, unsigned int type);
	void destroy();

	bool load(const char *fname, unsigned int type);
};

#define VSDR(s)		s, GL_VERTEX_SHADER
#define FSDR(s)		s, GL_FRAGMENT_SHADER
#define PSDR(s)		FSDR(s)
#define GSDR(s)		s, GL_GEOMETRY_SHADER
#define TCSDR(s)	s, GL_TESS_CONTROL_SHADER
#define TESDR(s)	s, GL_TESS_EVALUATION_SHADER

class ShaderProg {
private:
	unsigned int prog;
	mutable bool must_link;
	std::vector<Shader*> shaders;

    struct StateLocCache { int sidx, loc; std::string name; };
	/** a cache of all st_ prefixed uniform locations and their corresponding
	 * index in the global uniform state vector (see unistate.h)
	 */
	mutable std::vector<StateLocCache> stloc_cache;

	void cache_state_uniforms() const;
	void setup_state_uniforms() const;

public:
	static ShaderProg *current;

	ShaderProg();
	~ShaderProg();

	/// returns the OpenGL object id for this shader program
	unsigned int get_id() const;

	/** takes a series of shaders, and constructs a program object by linking
	 * them together. Terminate with a null pointer (don't use 0!) */
	bool create(Shader *sdr, ...);
	/// same as above, but with a va_list instead of variable arguments.
	bool create(Shader *sdr, va_list ap);
	/** takes two shaders (vertex and pixel) and constructs a program object by
	 * linking them together. Either one can be null. */
	bool create(Shader *vsdr, Shader *psdr);

	/** takes a series of shader source/shader type pairs and constructs a program
	 * object by linking them together. Terminate with a null pointer (don't use 0!)
	 * You can use the VSDR, PSDR, GSDR, TCSDR, TESDR convenience macros for passing
	 * the pairs.
	 * Example: create(VSDR(vsrc0), VSDR(vsrc1), PSDR(psrc), NULL);
	 */
	bool create(const char *src, unsigned int type, ...);
	/// same as above, but with a va_list instead of variable arguments.
	bool create(const char *src, unsigned int type, va_list ap);
	/** takes two shaders source strings (vertex and pixel) and constructs
	 * a program object by linking them together. Either one can be null. */
	bool create(const char *vsrc, const char *psrc);

	void destroy();

	/** takes a series of shader filename/shader type pairs, loads the shaders and
	 * constructs a program object by linking them together. Terminate with a null
	 * pointer (don't use 0!). You can use the VSDR, PSDR, GSDR, TCSDR, TESDR convenience
	 * macros for passing the pairs.
	 * Example: load(VSDR("vsdr1.glsl"), VSDR("vsdr2.glsl"), PSDR("pixel.glsl"), NULL);
	 */
	bool load(const char *fname, unsigned int type, ...);
	/// same as above, but with a va_list instead of variable arguments.
	bool load(const char *fname, unsigned int type, va_list ap);
	/** takes the filenames of two shader files (vertex and pixel), loads them and
	 * constructs a program object by linking them together. Either one can be null */
	bool load(const char *vsrc, const char *psrc);

	void add_shader(Shader *sdr);
	bool link() const;

	void bind() const;

	int get_attrib_location(const char *name) const;
	void set_attrib_location(const char *name, int loc) const;

	int get_uniform_location(const char *name) const;

	bool set_uniform(int loc, int val) const;
	bool set_uniform(int loc, float val) const;
	bool set_uniform(int loc, const Vector2 &v) const;
	bool set_uniform(int loc, const Vector3 &v) const;
	bool set_uniform(int loc, const Vector4 &v) const;
	bool set_uniform(int loc, const Matrix3x3 &m) const;
	bool set_uniform(int loc, const Matrix4x4 &m) const;

	bool set_uniform(const char *name, int val) const;
	bool set_uniform(const char *name, float val) const;
	bool set_uniform(const char *name, const Vector2 &v) const;
	bool set_uniform(const char *name, const Vector3 &v) const;
	bool set_uniform(const char *name, const Vector4 &v) const;
	bool set_uniform(const char *name, const Matrix3x3 &m) const;
	bool set_uniform(const char *name, const Matrix4x4 &m) const;

	friend void setup_unistate(const ShaderProg*);
};

class ShaderSet : public DataSet<Shader*> {
private:
	unsigned int type;

public:
	ShaderSet(unsigned int type);
};

#endif	// SHADER_H_

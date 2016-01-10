#ifndef MESH_H_
#define MESH_H_
#include <string>
#include <vector>
#include "vmath.h"

// fixed attribute locations, use SdrProg::set_attrib_loc() after the shader creation
enum {
	MESH_ATTR_VERTEX,
	MESH_ATTR_NORMAL,
	MESH_ATTR_TANGENT,
	MESH_ATTR_UV
};

struct Vertex 
{
	Vector3 pos;
	Vector3 normal;
	Vector3 tangent;
	float u, v;
	Vertex();
};

struct Face 
{
	unsigned int vidx[3];
	Face();
};

class Mesh 
{
private:
	std::string name;
	std::vector<Vertex> verts;
	std::vector<Face> faces;

	// defined as mutable, because we want to set them from create_buffers (which is const)
	mutable unsigned int vbo, ibo;

	void create_buffers() const;

public:
	Mesh();
	~Mesh();

	void set_name(const char* name);
	void set_vertex_data(int num_verts, const Vertex *data);
	void set_face_data(int num_faces, const Face *data);

	void add_vertex(const Vertex &v);
	void add_index(const Face &face);

	const char* get_name() const;
	Vertex *get_vertex_data();
	Face *get_face_data();
	Vertex *get_vertex_by_index(unsigned int idx);
	Face *get_face_by_index(unsigned int idx);
    unsigned int get_vertex_count();
    unsigned int get_face_count();

	bool has_name();
	bool has_verts();
	bool has_faces();

	void render() const;
};

#endif // MESH_H_

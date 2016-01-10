#include "mesh.h"
#include "GL/glew.h"
#include <iostream>

Vertex::Vertex()
{
	pos.x=pos.y=pos.z=0;
	normal.x=normal.y=normal.z=0;
	tangent.x=tangent.y=tangent.z=0;
	u=v=0;
}

Face::Face()
{
	vidx[0]=vidx[1]=vidx[2]=0;
}

Mesh::Mesh()
{
	vbo=0;
	ibo=0;
}

Mesh::~Mesh()
{
	glDeleteBuffers(1,&vbo);
	glDeleteBuffers(1,&ibo);
}

void Mesh::add_vertex(const Vertex &v)
{
	verts.push_back(v);
}

void Mesh::add_index(const Face &face)
{
	faces.push_back(face);
}

void Mesh::create_buffers() const
{
	glGenBuffers(1,&vbo);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(Vertex),&verts[0],GL_STATIC_DRAW);

	glGenBuffers(1,&ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,faces.size()*sizeof(Face),&faces[0],GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

void Mesh::set_name(const char* name)
{
	this->name=name;
}

void Mesh::set_vertex_data(int num_verts, const Vertex *data)
{
	verts.resize(num_verts);
	memcpy(&verts[0],data,num_verts*sizeof(Vertex));
}

void Mesh::set_face_data(int num_faces, const Face *data)
{
	faces.resize(num_faces);
	memcpy(&faces[0],data,num_faces*sizeof(Face));
}

const char* Mesh::get_name() const
{
	return name.c_str();
}

Vertex* Mesh::get_vertex_data()
{
	if(verts.empty())
	{
		std::cout<<"Warning: Mesh.cpp--> Vertex vector is empty!"<<std::endl;
		return 0;
	}
	return &verts[0];
}

Face* Mesh::get_face_data()
{
	if(faces.empty())
	{
		std::cout<<"Warning: Mesh.cpp--> Faces vector is empty!"<<std::endl;
		return 0;
	}
	return &faces[0];
}

Vertex* Mesh::get_vertex_by_index(unsigned int idx)
{
	if (idx > get_vertex_count())
		return NULL;

	return &verts[idx];
}

Face* Mesh::get_face_by_index(unsigned int idx)
{
	if (idx > get_face_count())
		return NULL;

	return &faces[idx];
}

unsigned int Mesh::get_vertex_count()
{
    return verts.size();
}

unsigned int Mesh::get_face_count()
{
    return faces.size();
}

bool Mesh::has_name()
{
	if(!name.empty())
	{
		return true;
	}
	return false;
}

bool Mesh::has_verts()
{
	if(!verts.empty())
	{
		return true;
	}
	return false;
}

bool Mesh::has_faces()
{
	if(!faces.empty())
	{
		return true;
	}
	return false;
}

void Mesh::render() const
{
	if(!vbo || !ibo)
	{
		create_buffers();
	}

	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);

	glEnableVertexAttribArray(MESH_ATTR_VERTEX);
	glVertexAttribPointer(MESH_ATTR_VERTEX,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),0);

	glEnableVertexAttribArray(MESH_ATTR_NORMAL);
	glVertexAttribPointer(MESH_ATTR_NORMAL,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));

	glEnableVertexAttribArray(MESH_ATTR_TANGENT);
	glVertexAttribPointer(MESH_ATTR_TANGENT,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));

	glEnableVertexAttribArray(MESH_ATTR_UV);
	glVertexAttribPointer(MESH_ATTR_UV,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(9*sizeof(float)));

	glDrawElements(GL_TRIANGLES,3*faces.size(),GL_UNSIGNED_INT,0);

	glDisableVertexAttribArray(MESH_ATTR_VERTEX);
	glDisableVertexAttribArray(MESH_ATTR_NORMAL);
	glDisableVertexAttribArray(MESH_ATTR_TANGENT);
	glDisableVertexAttribArray(MESH_ATTR_UV);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

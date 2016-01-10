#ifndef CAMERA_H_
#define CAMERA_H_
#include "xform_node.h"

class Camera:public XFormNode
{
protected:
	mutable Matrix4x4 camera_matrix;
	float near_clip_plane;
	float far_clip_plane;
	float horizontal_fov;

public:
	Camera();
	~Camera();
	virtual const Matrix4x4& get_matrix() const;
	const Camera *get_camera() const;
	const float get_near_clip_plane() const; //remove const from floats
	const float get_far_clip_plane() const;
	const float get_horizontal_fov() const;
	void set_near_clip_plane(const float near_plane);
	void set_far_clip_plane(const float far_plane);
	void set_horizontal_fov(const float fov);
	virtual void set_camera(long time = 0);
    XFormNode *clone();
};

class TargetCamera:public Camera
{
private:
	XFormNode *target;
	Vector3 up_vector;
	Vector3 lookat_vector;

public:
	TargetCamera();
	~TargetCamera();
	const Matrix4x4& get_matrix() const; // get_position, target->get_position, kai tha ftiaxnei lookat matrix
	const Vector3& get_up_vector() const;
	const Vector3& get_lookat_vector() const;
	void set_camera(long time = 0);
	void set_target(XFormNode *tar);
	void calc_target_pos();
	void set_up_vector(const Vector3 &up_vec);
	void set_up_vector(const float x,const float y,const float z);
	void set_lookat_vector(const Vector3 &lookat);
	void set_lookat_vector(const float x,const float y,const float z);
};

#endif //CAMERA_H_

#include "camera.h"
#include "unistate.h"
#include <iostream>

//DEBUG
#include "opengl.h"

Camera::Camera()
{
	camera_matrix.reset_identity();
    set_node_type(NODE_CAMERA);
}

Camera::~Camera()
{
}

const Matrix4x4& Camera::get_matrix() const
{
	camera_matrix=XFormNode::get_matrix();
	camera_matrix.inverse();
	return camera_matrix;
}

const Camera* Camera::get_camera() const
{
	return this;
}

const float Camera::get_near_clip_plane() const
{
	return near_clip_plane;
}

const float Camera::get_far_clip_plane() const
{
	return far_clip_plane;
}

const float Camera::get_horizontal_fov() const
{
	return horizontal_fov;
}

void Camera::set_near_clip_plane(const float near_plane)
{
	near_clip_plane=near_plane;
}

void Camera::set_far_clip_plane(const float far_plane)
{
	far_clip_plane=far_plane;
}

void Camera::set_horizontal_fov(const float fov)
{
	horizontal_fov=fov;
}

void Camera::set_camera(long time)
{
	calc_matrix(time);
	//std::cout<<get_matrix()<<std::endl;
	set_view_matrix(get_matrix());
}

XFormNode* Camera::clone()
{
    return new Camera(*this);
}

//--------------------------------Target Camera------------------------------------
TargetCamera::TargetCamera()
{
	target=NULL;
	up_vector=Vector3(0,1,0);
	lookat_vector=Vector3(0,0,1);
}

TargetCamera::~TargetCamera()
{
	if(target)
		delete target;
}

const Matrix4x4& TargetCamera::get_matrix() const
{

	camera_matrix.reset_identity();
	camera_matrix.set_lookat(get_position(),target->get_position(),get_up_vector());
	return camera_matrix;			
}

const Vector3& TargetCamera::get_up_vector() const
{
	return up_vector;
}

const Vector3& TargetCamera::get_lookat_vector() const
{
	return lookat_vector;
}

void TargetCamera::set_camera(long time)
{
	set_view_matrix(get_matrix());
	set_unistate("st_view_matrix3",Matrix3x3(get_matrix()));

    //DEBUG
    Matrix4x4 view = get_matrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadTransposeMatrixf(view[0]);
}

void TargetCamera::set_target(XFormNode *tar)
{
	target=tar;
}

void TargetCamera::calc_target_pos()
{
	//target position=camera pos+lookat vector.
	Vector3 tar_pos=get_position()+lookat_vector;
	target->set_position(tar_pos);
}

void TargetCamera::set_up_vector(const Vector3 &up_vec)
{
	up_vector.x=up_vec.x;
	up_vector.y=up_vec.y;
	up_vector.z=up_vec.z;
}

void TargetCamera::set_up_vector(const float x, const float y, const float z)
{
	up_vector.x=x;
	up_vector.y=y;
	up_vector.z=z;
}

void TargetCamera::set_lookat_vector(const Vector3 &lookat)
{
	lookat_vector.x=lookat.x;
	lookat_vector.y=lookat.y;
	lookat_vector.z=lookat.z;
}

void TargetCamera::set_lookat_vector(const float x, const float y, const float z)
{
	lookat_vector.x=x;
	lookat_vector.y=y;
	lookat_vector.z=z;
}

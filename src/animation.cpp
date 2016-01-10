#include "animation.h"

//struct contructors-------------------------
KeyframeVec3::KeyframeVec3()
{
    v.x=v.y=v.z=0;
    time=0;
}

KeyframeVec3::KeyframeVec3(const Vector3 &v, long time)
{
    this->v = v;
    this->time = time;
}

KeyframeQuat::KeyframeQuat()
{
    time=0;
}

KeyframeQuat::KeyframeQuat(const Quaternion &r, long time)
{
    this->r = r;
    this->time = time;
}

//-------------------------------------------

//struct operator overloads------------------------------------
bool KeyframeVec3::operator <(const KeyframeVec3 &key2) const
{
    return time<key2.time;
}

bool KeyframeQuat::operator <(const KeyframeQuat &key2) const
{
    return time<key2.time;
}
//-------------------------------------------------------------


//Animation class functions------------------------------------
Animation::Animation()
{
    name = "";
    loop = true;
    anim_speed = 1;

    KeyframeVec3 def_pos = KeyframeVec3(Vector3() , 0);
    position.push_back(def_pos);

    KeyframeQuat def_rot = KeyframeQuat(Quaternion() , 0);
    rotation.push_back(def_rot);

    KeyframeVec3 def_scale = KeyframeVec3(Vector3(1.0 , 1.0 , 1.0) , 0);
    scaling.push_back(def_scale);
}

Animation::Animation(const std::string &name, bool loop_state)
{
    set_name(name);
    set_looping(loop_state);
    anim_speed = 1;

    KeyframeVec3 def_pos = KeyframeVec3(Vector3() , 0);
    position.push_back(def_pos);

    KeyframeQuat def_rot = KeyframeQuat(Quaternion() , 0);
    rotation.push_back(def_rot);

    KeyframeVec3 def_scale = KeyframeVec3(Vector3(1.0 , 1.0 , 1.0) , 0);
    scaling.push_back(def_scale);
}

Animation::~Animation()
{

}

void Animation::set_name(const std::string &name)
{
    this->name = name;
}

void Animation::set_looping(bool loop_state)
{
    loop = loop_state;
}

const std::string &Animation::get_name() const
{
    return name;
}

const KeyframeVec3& Animation::get_position_keyframe(unsigned int idx) const
{
    return position[idx];
}

const KeyframeQuat& Animation::get_rotation_keyframe(unsigned int idx) const
{
    return rotation[idx];
}

const KeyframeVec3& Animation::get_scaling_keyframe(unsigned int idx) const
{
    return scaling[idx];
}

unsigned int Animation::get_position_count() const
{
    return position.size();
}

unsigned int Animation::get_rotation_count() const
{
    return rotation.size();
}

unsigned int Animation::get_scaling_count() const
{
    return scaling.size();
}

void Animation::set_anim_speed(float speed)
{
    anim_speed = speed;
}

float Animation::get_anim_speed() const
{
    return anim_speed;
}

bool Animation::position_is_empty() const
{
    if(position.empty())
        return true;
    return false;
}

bool Animation::rotation_is_empty() const
{
    if(rotation.empty())
        return true;
    return false;
}

bool Animation::scaling_is_empty() const
{
    if(scaling.empty())
        return true;
    return false;
}

bool Animation::is_loop_enabled() const
{
    return loop;
}

void Animation::add_position_keyframe(const KeyframeVec3 &pos)
{
    position.push_back(pos);
}

void Animation::add_rotation_keyframe(const KeyframeQuat &rot)
{
    rotation.push_back(rot);
}

void Animation::add_scaling_keyframe(const KeyframeVec3 &scaling)
{
    this->scaling.push_back(scaling);
}

void Animation::replace_position_keyframe(const KeyframeVec3 &pos , unsigned int idx)
{
    position[idx] = pos;
}

void Animation::replace_rotation_keyframe(const KeyframeQuat &rot, unsigned int idx)
{
    rotation[idx] = rot;
}

void Animation::replace_scaling_keyframe(const KeyframeVec3 &scaling, unsigned int idx)
{
    this->scaling[idx] = scaling;
}

void Animation::clear_all_keyframes()
{
    if(!position.empty())
    {
        position.clear();
    }

    if(!rotation.empty())
    {
        rotation.clear();
    }

    if(!scaling.empty())
    {
        scaling.clear();
    }
}

void Animation::sort_position_keyframes()
{
    std::sort(position.begin() , position.end());
}

void Animation::sort_rotation_keyframes()
{
    std::sort(rotation.begin() , rotation.end());
}

void Animation::sort_scaling_keyframes()
{
    std::sort(scaling.begin() , scaling.end());
}

//--------------------------------------------------------------

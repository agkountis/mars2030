#ifndef ANIMATION_H
#define ANIMATION_H
#include <string>
#include <vector>
#include <algorithm>
#include "vmath.h"

#define DEFAULT_VALUES_IDX 0

struct KeyframeVec3
{
    Vector3 v;
    long time;
    KeyframeVec3();
    KeyframeVec3(const Vector3 &v , long time);
    bool operator <(const KeyframeVec3 &key2) const;
};

struct KeyframeQuat
{
    Quaternion r;
    long time;
    KeyframeQuat();
    KeyframeQuat(const Quaternion &r , long time);
    bool operator <(const KeyframeQuat &key2) const;
};

class Animation
{
private:
    std::string name;
    std::vector<KeyframeVec3> position;
    std::vector<KeyframeQuat> rotation;
    std::vector<KeyframeVec3> scaling;
    bool loop;
    float anim_speed;

public:
    Animation();
    Animation(const std::string &name, bool loop_state);
    ~Animation();

    void set_name(const std::string &name);
    void set_looping(bool loop_state);

    const std::string &get_name() const;

    const KeyframeVec3 &get_position_keyframe(unsigned int idx) const;
    const KeyframeQuat &get_rotation_keyframe(unsigned int idx) const;
    const KeyframeVec3 &get_scaling_keyframe(unsigned int idx) const;

    unsigned int get_position_count() const; //returns the size of the position vector.
    unsigned int get_rotation_count() const; //returns the size of the rotation vector.
    unsigned int get_scaling_count() const;  //return the size of the scaling vector.

    void set_anim_speed(float speed);
    float get_anim_speed() const;

    bool position_is_empty() const;
    bool rotation_is_empty() const;
    bool scaling_is_empty() const;
    bool is_loop_enabled() const;

    void add_position_keyframe(const KeyframeVec3 &pos);
    void add_rotation_keyframe(const KeyframeQuat &rot);
    void add_scaling_keyframe(const KeyframeVec3 &scaling);

    void replace_position_keyframe(const KeyframeVec3 &pos, unsigned int idx);
    void replace_rotation_keyframe(const KeyframeQuat &rot, unsigned int idx);
    void replace_scaling_keyframe(const KeyframeVec3 &scaling, unsigned int idx);

    void clear_all_keyframes();

    void sort_position_keyframes();
    void sort_rotation_keyframes();
    void sort_scaling_keyframes();
};



#endif // ANIMATION_H

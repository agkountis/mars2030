#ifndef XFORM_NODE_H_
#define XFORM_NODE_H_
#include "animation.h"

enum NodeTypes {
    NODE_NULL, NODE_OBJ, NODE_ENT, NODE_PSYS, NODE_LIGHT, NODE_CAMERA
};

#define RENDER_ALL 0xffffffff
#define RENDER_OBJ (1 << NODE_OBJ)
#define RENDER_PSYS (1 << NODE_PSYS)
#define RENDER_LIGHT (1 << NODE_LIGHT)
#define RENDER_CAMERA (1 << NODE_CAMERA)


class XFormNode
{
private:
	std::string name;
    std::vector<Animation*> anims;
    unsigned int curr_anim;
    long anim_start_time;

	Matrix4x4 xform_mat;
	Matrix4x4 local_mat;

	XFormNode *parent;
	std::vector<XFormNode*> children;

    unsigned int node_type;

public:
	XFormNode();
    XFormNode(const std::string &name);
    XFormNode(const XFormNode &node); //deep copy constructor
    XFormNode &operator = (const XFormNode &node); // when deep copying always overload the = operator.
	virtual ~XFormNode();

    virtual void set_name(const std::string &name);
    virtual const std::string &get_name() const;

	virtual void set_position(const Vector3 &pos , long time = 0);
    virtual void set_rotation(const Vector3 &axis, float angle , long time = 0);
    virtual void set_rotation(const Quaternion &rot , long time = 0);
	virtual void set_scaling(const Vector3 &scale , long time = 0);
	virtual void set_matrix(const Matrix4x4 &mat);
    void set_node_type(unsigned int node_type);

    virtual XFormNode* get_parent() const;
    virtual XFormNode* get_child(unsigned int idx) const;
    virtual unsigned int get_children_count() const;
    virtual Vector3 get_position(long time = 0) const;
    virtual Quaternion get_rotation(long time = 0) const;
    virtual Vector3 get_scaling(long time = 0) const;
	virtual const Matrix4x4& get_matrix() const;
    unsigned int get_curr_anim_idx() const;
    long get_anim_start_time() const;
    unsigned int get_node_type() const;
    bool get_anim_looping_state() const;

	virtual void add_child(XFormNode *c);
    virtual void remove_child(XFormNode *c);
    virtual void set_parent(XFormNode *p);
    virtual void calc_matrix(long time = 0);
    virtual void set_anim_looping(bool state);

    /*calculates the martix of a whole hierarchy of nodes.*/
    void calc_hierarchy_mat();

    /*Returns the idx of the animation in the vector if the name matches.
     Else returns -1 for error handling*/
    virtual int lookup_animation(const char *name) const;

    void add_animation(Animation *anim);

    /*sets the animation idx to the specified index and assings the start time
     *affects the whole hierarchy if one exists.*/
    virtual void start_animation(int idx, long anim_start_time = 0);

    long get_max_anim_time();
    long get_max_anim_time(unsigned int anim_idx);

    void set_curr_anim_speed(float speed);
    void set_anim_speed(unsigned int idx, float speed);

    void set_hierarchy_anim_speed(float speed);
    void set_hierarchy_anim_speed(unsigned int idx, float speed);

    float get_anim_speed() const;
    virtual XFormNode *clone();

    virtual void render(unsigned int render_mask, long time = 0);
};

#endif	// XFORM_NODE_H_

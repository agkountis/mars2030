#include "xform_node.h"
#include "unistate.h"
#include "game.h"
#include <algorithm>

XFormNode::XFormNode()
{
    curr_anim = 0;
    name = "";
    parent = NULL;
    anim_start_time = 0;
    Animation *anim = new Animation;
    anim->set_name("default");
    add_animation(anim);
    node_type = NODE_NULL;
}

XFormNode::XFormNode(const std::string &name)
{
    curr_anim = 0;
    this->name = name;
    parent = NULL;
    anim_start_time = 0;
    add_animation(new Animation);
    node_type = NODE_NULL;
}

XFormNode::XFormNode(const XFormNode &node)
{
    name = node.name;
    curr_anim = node.curr_anim;
    anim_start_time = node.anim_start_time;
    parent = NULL;
    node_type = node.node_type;

    anims.resize(node.anims.size());

    for(unsigned int i = 0 ; i < node.anims.size() ; i++){
        anims[i] = new Animation(*node.anims[i]);
    }

    unsigned int pos = name.find_first_of('_');

    std::string substr1 = name.substr(0, pos);

    if(substr1.compare("NUC") == 0){
        if(!game::engine::emitter_configs.empty() && !game::engine::physics_sims.empty()){
            NucEmitter *em;
            NucPhysics *ph;

            unsigned int pos2 = name.find_first_of('$') - 1;
            std::string substr2 = name.substr(pos + 1, pos2 - pos - 1);

            unsigned int fullstop = substr2.find('.');

            std::string emitter_name = substr2.substr(0, fullstop);
            std::string physics_name = substr2.substr(fullstop + 1);

            em = new NucEmitter;

            unsigned int em_idx = game::engine::emitter_config_idx_by_name[emitter_name];
            unsigned int ph_idx = game::engine::physics_sim_idx_by_name[physics_name];

            em->set_emitter_config(game::engine::emitter_configs[em_idx]);

            ph = game::engine::physics_sims[ph_idx];

            em->set_physics_simulator(ph);
            em->set_renderer(game::engine::ps_r);
            em->init();

            add_child(em);
            game::engine::nuc_manager->add_emitter(em);
        }
    }

    for(unsigned int i = 0 ; i < node.get_children_count() ; i++){
        add_child(node.get_child(i)->clone());
    }
}

XFormNode& XFormNode::operator = (const XFormNode &node) //not sure if its correct.
{
    if(!anims.empty()){
        std::vector<Animation*>::iterator it = anims.begin();

        for(unsigned int i = 0 ; i < anims.size() ; i++){
            delete anims[i];
            it = anims.erase(it);
        }
    }

    name = node.name;
    curr_anim = node.curr_anim;
    anim_start_time = node.anim_start_time;
    node_type = node.node_type;

    anims.resize(node.anims.size());

    for(unsigned int i = 0 ; i < node.anims.size() ; i++){
        anims[i] = new Animation(*node.anims[i]);
    }

    for(unsigned int  i = 0 ; i < node.get_children_count() ; i++){
        add_child(node.get_child(i)->clone());
    }

    return *this;
}

XFormNode::~XFormNode()
{
    if(!anims.empty()){
        for(unsigned int i = 0 ; i < anims.size() ; i++){
            delete anims[i];
        }
    }

    if(get_children_count()){
        for(unsigned int i = 0 ; i < children.size() ; i++){
            XFormNode *c = get_child(i);

            if(c->node_type == NODE_PSYS){
                remove_child(c);
                c->set_parent(NULL);
            }
        }
    }
}

void XFormNode::set_name(const std::string &name)
{
    this->name = name;
}

const std::string &XFormNode::get_name() const
{
	return name;
}

void XFormNode::set_position(const Vector3 &pos, long time)
{
    KeyframeVec3 tmp;
    Animation *anim = anims[curr_anim];

    if(!anim->position_is_empty()){
        for(unsigned int i = 0 ; i < anim->get_position_count() ; i++){
            KeyframeVec3 anim_keyframe = anim->get_position_keyframe(i);

            if(anim_keyframe.time == time){
                anim->replace_position_keyframe(KeyframeVec3(pos , time) , i);
                return;
            }
        }
    }

    tmp.time = time;
    tmp.v = pos;
    anim->add_position_keyframe(tmp);
    anim->sort_position_keyframes();
}

void XFormNode::set_rotation(const Vector3 &axis, float angle, long time)
{
    KeyframeQuat tmp;
    Quaternion rot;
    Animation *anim = anims[curr_anim];

    rot.set_rotation(axis.normalized() , angle);
    if(!anim->rotation_is_empty()){
        for(unsigned int i = 0 ; i < anim->get_rotation_count() ; i++){
            KeyframeQuat anim_keyframe = anim->get_rotation_keyframe(i);

            if(anim_keyframe.time == time){
                anim->replace_rotation_keyframe(KeyframeQuat(rot,time) , i);
                return;
            }
        }
    }

    tmp.time = time;
    tmp.r = rot;
    anim->add_rotation_keyframe(tmp);
    anim->sort_rotation_keyframes();
}

void XFormNode::set_rotation(const Quaternion &a , long time)
{
    KeyframeQuat tmp;
    Animation *anim = anims[curr_anim];

    if(!anim->rotation_is_empty()){
        for(unsigned int i = 0 ; i < anim->get_rotation_count() ; i++){
            KeyframeQuat anim_keyframe = anim->get_rotation_keyframe(i);

            if(anim_keyframe.time == time){
                anim->replace_rotation_keyframe(KeyframeQuat(a , time) , i);
                return;
            }
        }
    }

    tmp.time = time;
    tmp.r = a;
    anim->add_rotation_keyframe(tmp);
    anim->sort_rotation_keyframes();
}

void XFormNode::set_scaling(const Vector3 &scale, long time)
{
    KeyframeVec3 tmp;
    Animation *anim = anims[curr_anim];

    if(!anim->scaling_is_empty()){
        for(unsigned int i = 0 ; i < anim->get_scaling_count() ; i++){
            KeyframeVec3 anim_keyframe = anim->get_scaling_keyframe(i);

            /*if the scaling keyframe already exists replace it.*/
            if(anim_keyframe.time == time){
                anim->replace_scaling_keyframe(KeyframeVec3(scale , time) , i);
                return;
            }
        }
    }

    tmp.time = time;
    tmp.v = scale;
    anim->add_scaling_keyframe(tmp);
    anim->sort_scaling_keyframes();
}

void XFormNode::set_matrix(const Matrix4x4 &mat)
{
    local_mat = mat;
}

void XFormNode::set_node_type(unsigned int node_type)
{
    this->node_type = node_type;
}

XFormNode* XFormNode::get_parent() const
{
	if(parent)
		return parent;
	return 0;
}

XFormNode* XFormNode::get_child(unsigned int idx) const
{
	if(!children.empty())
		return children[idx];
	std::cout<<"XFormNode::get_child(unsigned int idx)->>> Children vector is empty!!!"
		<<std::endl;
	return 0;
}

unsigned int XFormNode::get_children_count() const
{
	return (unsigned int)children.size();
}

Vector3 XFormNode::get_position(long time) const
{
    Animation *anim = anims[curr_anim];

    /*if the <vector> is empty return a 0,0,0 vector.*/
    if(anim->position_is_empty()){
        return Vector3();
	}

    KeyframeVec3 first_keyframe = anim->get_position_keyframe(0);
    KeyframeVec3 last_keyframe = anim->get_position_keyframe(anim->get_position_count() - 1);

    long start_time = first_keyframe.time;  //start time of the animation.
    long full_interval = last_keyframe.time-start_time; //full duratition of the animation.

    //animation loop algorithm----------
    if(full_interval && anim->is_loop_enabled()){
        time -= start_time;
        time %= full_interval;
        time += start_time;
	}
	//----------------------------------

    /*if global time is less than the 1st keyframe's time return the 1st keyframe.*/
    if(time < first_keyframe.time){
        return first_keyframe.v;
	}

    /*if global time is greater or equal with the time of the last keyframe return the last keyframe*/
    if(time >= last_keyframe.time){
        return last_keyframe.v;
	}

	long interval;
	double t;

    for(unsigned int i = 0 ; i < anim->get_position_count() - 1 ; i++){
        KeyframeVec3 current = anim->get_position_keyframe(i);
        KeyframeVec3 next = anim->get_position_keyframe(i+1);

        if( (time >= current.time) && (time <= next.time) ){
            Vector3 tmp_pos;

            /*calculation of the interval between keyframes.*/
            interval = next.time - current.time;

            /*calculation of the interpolation parameter t.*/
            t= (double)(time - current.time) / (double)interval;

			float x1,x2,y1,y2,z1,z2;
            x1 = current.v.x;
            x2 = next.v.x;
            y1 = current.v.y;
            y2 = next.v.y;
            z1 = current.v.z;
            z2 = next.v.z;

            /*x = a + (b - a) * t ->>>linear interpolation*/
            tmp_pos.x=x1 + (x2 - x1) * t;
            tmp_pos.y=y1 + (y2 - y1) * t;
            tmp_pos.z=z1 + (z2 - z1) * t;

            return tmp_pos;
		}
	}

    /*if nothing of the above happened it means the keyrame doesn't exist so just return a 0,0,0 vector.*/
    std::cout<<"Error: Position Keyframe not found!!!"<<std::endl;
    return Vector3();
}

XFormNode* XFormNode::clone()
{
    return new XFormNode(*this);
}

Quaternion XFormNode::get_rotation(long time) const
{
    Animation *anim = anims[curr_anim];

    /*if the <vector> is empty return an identity quaternion.*/
    if(anim->rotation_is_empty()){
        return Quaternion();
	}

    KeyframeQuat first_keyframe = anim->get_rotation_keyframe(0);
    KeyframeQuat last_keyframe = anim->get_rotation_keyframe(anim->get_rotation_count() - 1);

    long start_time=first_keyframe.time; //start time of the animation.
    long full_interval=last_keyframe.time-start_time;//full duration of the animation.

    //animation loop algorithm----------
    if(full_interval && anim->is_loop_enabled()){
        time -= start_time;
        time %= full_interval;
        time += start_time;
	}
    //----------------------------------

    if(time < first_keyframe.time){
        return first_keyframe.r;
	}

    if(time >= last_keyframe.time){
        return last_keyframe.r;
	}

	long interval;
	double t;

    for(unsigned int i = 0 ; i < anim->get_rotation_count() - 1 ; i++){
        KeyframeQuat current = anim->get_rotation_keyframe(i);
        KeyframeQuat next = anim->get_rotation_keyframe(i + 1);

        if( (time >= current.time) && (time <= next.time) ){
            Quaternion tmp_rot;

            /*calculation of the interval between keyframes.*/
            interval = next.time - current.time;

            /*calculation of the interpolated parameter t.*/
            t = (double)(time - current.time) / (double)interval;

            /*Spherical linear interpolation--->>>SLERP!(cause we have rotations).*/
            tmp_rot = slerp(current.r , next.r , t);

            return tmp_rot;
		}
	}
	std::cout<<"Error: Rotation Keyframe not found!!!"<<std::endl;
    return Quaternion();
}

Vector3 XFormNode::get_scaling(long time) const
{
    Animation *anim = anims[curr_anim];

    /*if the <vector> is empty return a 0,0,0 vector.*/
    if(anim->scaling_is_empty()){
        return Vector3(1.0 , 1.0 , 1.0);
	}

    KeyframeVec3 first_keyframe = anim->get_scaling_keyframe(0);
    KeyframeVec3 last_keyframe = anim->get_scaling_keyframe(anim->get_scaling_count() - 1);

    long start_time = first_keyframe.time; //start time of the animation.
    long full_interval = last_keyframe.time-start_time; //full duration of the animation.

    //animation loop algorithm----------
    if(full_interval && anim->is_loop_enabled()){
        time -= start_time;
        time %= full_interval;
        time += start_time;
	}
	//----------------------------------

    /*if global time is less than the 1st keyframe's time return the 1st keyframe.*/
    if(time < first_keyframe.time) {
        return first_keyframe.v;
	}

    /*if global time is greater or equal with the time of the last keyframe,
     *  return the last keyframe.*/
    if(time >= last_keyframe.time){
        return last_keyframe.v;
	}

	long interval;
	double t;

    for(unsigned int i = 0 ; i < anim->get_scaling_count() - 1 ; i++){
        KeyframeVec3 current = anim->get_scaling_keyframe(i);
        KeyframeVec3 next = anim->get_scaling_keyframe(i + 1);

        if( (time >= current.time) && (time <= next.time) ){
            Vector3 tmp_scaling;

            interval = next.time - current.time; //calculation of the interval between keyframes.
            t = (double)(time - current.time) / (double)interval;//calculation of the interpolation parameter t.

			float x1,x2,y1,y2,z1,z2;
            x1 = current.v.x;
            x2 = next.v.x;
            y1 = current.v.y;
            y2 = next.v.y;
            z1 = current.v.z;
            z2 = next.v.z;

			//x = a + (b - a) * t ->>>linear interpolation
            tmp_scaling.x=x1 + (x2 - x1) * t;
            tmp_scaling.y=y1 + (y2 - y1) * t;
            tmp_scaling.z=z1 + (z2 - z1) * t;

            return tmp_scaling;
		}
	}
	std::cout<<"Error: Scaling Keyframe not found!!!"<<std::endl;
    return Vector3(1.0 , 1.0 , 1.0);
}

const Matrix4x4& XFormNode::get_matrix() const
{
	return xform_mat;
}

unsigned int XFormNode::get_curr_anim_idx() const
{
    return curr_anim;
}

long XFormNode::get_anim_start_time() const
{
    return anim_start_time;
}

unsigned int XFormNode::get_node_type() const
{
    return node_type;
}

bool XFormNode::get_anim_looping_state() const
{
    return anims[curr_anim]->is_loop_enabled();
}

void XFormNode::add_child(XFormNode *c)
{
	children.push_back(c);
	children[children.size()-1]->parent=this;
}

void XFormNode::remove_child(XFormNode *c)
{
    if(!children.empty()){
        std::vector<XFormNode*>::iterator it = children.begin();

        while(it != children.end()){
            if(*it == c){
                it = children.erase(it);
            }
            else{
                it++;
            }
        }
    }
}

void XFormNode::set_parent(XFormNode *p)
{
    parent = p;
}

void XFormNode::calc_matrix(long time)
{
    Vector3 tmp_pos;
    Quaternion tmp_rot;
    Vector3 tmp_scaling;

    long t = (time - anim_start_time) * get_anim_speed();

    tmp_pos = get_position(t);
    tmp_rot = get_rotation(t).normalized();
    tmp_scaling = get_scaling(t);

    xform_mat.reset_identity();
    xform_mat.translate(tmp_pos);
    xform_mat.rotate(tmp_rot);
    xform_mat.scale(Vector4(tmp_scaling));
	xform_mat = local_mat * xform_mat;

	if(parent){
        xform_mat = parent->xform_mat * xform_mat;
	}
}

void XFormNode::calc_hierarchy_mat()
{
    calc_matrix();

    for(unsigned int i = 0 ; i < children.size() ; i++){
        children[i]->calc_hierarchy_mat();
    }
}

int XFormNode::lookup_animation(const char *name) const
{
    if(!anims.empty()){
        for(unsigned int i = 0 ; i < anims.size() ; i++){
            const std::string &tmp = anims[i]->get_name();

            if(strcmp(name, tmp.c_str()) == 0)
                return i;
        }
    }
    else{
        std::cout<<"The animation vector is empty!"<<std::endl;
        return -1;
    }

    std::cout<<"Could not find the specified animation!"<<std::endl;
    return -1;
}

void XFormNode::add_animation(Animation *anim)
{
    anims.push_back(anim);
}

void XFormNode::start_animation(int idx, long anim_start_time)
{
	if(idx < 0 || idx > anims.size() - 1)
		return;

    if(node_type == NODE_PSYS)    {
        curr_anim = 0;
    }
    else{
        curr_anim = idx;
        this->anim_start_time = anim_start_time;
    }

    if(this->get_children_count() > 0){
        for(unsigned int i = 0 ; i < get_children_count() ; i++){
            XFormNode *child = get_child(i);
            child->start_animation(idx , anim_start_time);
        }
    }
}

void XFormNode::set_anim_looping(bool state)
{
    anims[curr_anim]->set_looping(state);
}

long XFormNode::get_max_anim_time()
{
    long result = 0;
    long pos_max = 0;
    long rot_max = 0;
    long scale_max = 0;
    unsigned int idx = 0;

    if(node_type == NODE_PSYS)
        return 0;

    if(anims[curr_anim]->get_position_count() > 0){
        idx = anims[curr_anim]->get_position_count() - 1;
        pos_max = anims[curr_anim]->get_position_keyframe(idx).time;
    }

    if(anims[curr_anim]->get_rotation_count() > 0){
        idx = anims[curr_anim]->get_rotation_count() - 1;
        rot_max = anims[curr_anim]->get_rotation_keyframe(idx).time;
    }

    if(anims[curr_anim]->get_scaling_count() > 0){
        idx = anims[curr_anim]->get_scaling_count() - 1;
        scale_max = anims[curr_anim]->get_scaling_keyframe(idx).time;
    }

    result = (std::max)(pos_max, rot_max);
    result = (std::max)(result, scale_max);

    for(unsigned int i = 0 ; i < children.size() ; i++){
        long child_max = children[i]->get_max_anim_time();

        if(child_max > result){
            result = child_max;
        }
    }

    return result;
}

long XFormNode::get_max_anim_time(unsigned int anim_idx)
{
    long result = 0;
    long pos_max = 0;
    long rot_max = 0;
    long scale_max = 0;
    unsigned int idx = 0;

    if(anims.size() > anim_idx){
        if(anims[anim_idx]->get_position_count() > 0){
            idx = anims[anim_idx]->get_position_count() - 1;
            pos_max = anims[anim_idx]->get_position_keyframe(idx).time;
        }

        if(anims[anim_idx]->get_rotation_count() > 0){
            idx = anims[anim_idx]->get_rotation_count() - 1;
            rot_max = anims[anim_idx]->get_rotation_keyframe(idx).time;
        }

        if(anims[anim_idx]->get_scaling_count() > 0){
            idx = anims[anim_idx]->get_scaling_count() - 1;
            scale_max = anims[anim_idx]->get_scaling_keyframe(idx).time;
        }
    }

    result = (std::max)(pos_max, rot_max);
    result = (std::max)(result, scale_max);

    for(unsigned int i = 0 ; i < children.size() ; i++){
        long child_max = children[i]->get_max_anim_time(anim_idx);

        if(child_max > result){
            result = child_max;
        }
    }

    return result;
}

void XFormNode::set_curr_anim_speed(float speed)
{
    anims[curr_anim]->set_anim_speed(speed);
}

void XFormNode::set_anim_speed(unsigned int idx, float speed)
{
    if(idx <= anims.size() - 1){
        anims[idx]->set_anim_speed(speed);
    }
}

void XFormNode::set_hierarchy_anim_speed(float speed)
{
    set_curr_anim_speed(speed);

    for(unsigned int i = 0 ; i < children.size() ; i++){
        XFormNode *child = children[i];
        child->set_hierarchy_anim_speed(speed);
    }
}

void XFormNode::set_hierarchy_anim_speed(unsigned int idx, float speed)
{
    set_anim_speed(idx, speed);

    for(unsigned int i = 0 ; i < children.size() ; i++){
        XFormNode *child = children[i];
        child->set_hierarchy_anim_speed(idx, speed);
    }
}

float XFormNode::get_anim_speed() const
{
    return anims[curr_anim]->get_anim_speed();
}

void XFormNode::render(unsigned int render_mask, long time)
{
    calc_matrix(time);

    if(render_mask &(1 << node_type)){
        render(node_type, time);
    }

    for(unsigned int i = 0 ; i < children.size() ; i++){
        children[i]->render(render_mask, time);
    }
}

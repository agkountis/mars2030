#include "scene.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include "vmath.h"
#include "game.h"

static Mesh *load_mesh(const aiMesh *aimesh);
static Object *load_node(const aiScene *sc , const aiNode *n);
static Light *load_light(const aiLight *l , const std::vector<Object*> objs);
static Camera* load_camera(const aiCamera *cam , const std::vector<Object*> objs);
static void setup_node_animations(const aiScene *sc , XFormNode *node, const char *anim_name, bool loop_state);

static Vector3 ass_vector(const aiVector3D &v); //oh funny names ..funny names indeed...
static Quaternion ass_quat(const aiQuaternion &q);
static Matrix4x4 ass_mat(const aiMatrix4x4 &ai_m);
static long ass_time(const aiAnimation *ai_anim , double ai_time);
static void print_hierarchy(const aiNode *node);
static void delete_object_hierarchy(XFormNode *n);


static std::map<aiMesh*,Mesh*> mesh_by_aimesh;
static std::map<std::string,Object*> object_by_name;


Scene::Scene()
{
    active_camera = NULL;
    owns_meshes = true;

    max_anim_time = 0;
}

Scene::Scene(const Scene &sc)
{
    meshes = sc.meshes;
    owns_meshes = false;

    max_anim_time = 0;

    objects.resize(sc.objects.size());

    for(unsigned int i = 0 ; i < sc.objects.size() ; i++) {
        objects[i] = new Object(*sc.objects[i]);
    }


    lights.resize(sc.lights.size());

    for(unsigned int i = 0 ; i < sc.lights.size() ; i++) {
        lights[i] = new Light(*sc.lights[i]);
    }


    cameras.resize(sc.cameras.size());

    for(unsigned int  i = 0 ; i < sc.cameras.size() ; i++) {
        cameras[i] = new Camera(*sc.cameras[i]);
    }
}

Scene& Scene::operator = (const Scene &sc)
{
    if(!meshes.empty()) {
        
		if(owns_meshes) {
            std::vector<Mesh*>::iterator it = meshes.begin();

            for(unsigned int i = 0 ; i < meshes.size() ; i++) {
                delete meshes[i];
                it = meshes.erase(it);
            }
        }
    }

    if(!objects.empty()) {
        std::vector<Object*>::iterator it = objects.begin();

        for(unsigned int i = 0 ; i < objects.size() ; i++) {
            delete objects[i];
            it = objects.erase(it);
        }
    }

    if(!lights.empty()) {
        std::vector<Light*>::iterator it = lights.begin();

        for(unsigned int i = 0 ; i < lights.size() ; i++) {
            delete lights[i];
            it = lights.erase(it);
        }
    }

    if(!cameras.empty()) {
        std::vector<Camera*>::iterator it = cameras.begin();

        for(unsigned int i = 0 ; i < cameras.size() ; i++) {
            delete cameras[i];
            it = cameras.erase(it);
        }
    }


    meshes = sc.meshes;
    owns_meshes = false;

    objects.resize(sc.objects.size());

    for(unsigned int i = 0 ; i < sc.objects.size() ; i++) {
        objects[i] = new Object(*sc.objects[i]);
    }


    lights.resize(sc.lights.size());

    for(unsigned int i = 0 ; i < sc.lights.size() ; i++) {
        lights[i] = new Light(*sc.lights[i]);
    }


    cameras.resize(sc.cameras.size());

    for(unsigned int  i = 0 ; i < sc.cameras.size() ; i++) {
        cameras[i] = new Camera(*sc.cameras[i]);
    }

    return *this;
}

Scene::~Scene()
{
    if(owns_meshes) {
        for(unsigned int i = 0 ; i < meshes.size() ; i++) {
			if(meshes[i])
				delete meshes[i];
        }
    }

    for(unsigned int i = 0 ; i < objects.size() ; i++) {
		if(objects[i])
			delete_object_hierarchy(objects[i]);
	}

    for(unsigned int i = 0 ; i < lights.size() ; i++) {
		if(lights[i])
			delete lights[i];
    }

    for(unsigned int i = 0 ; i < cameras.size() ; i++) {
		if(cameras[i])
			delete cameras[i];
	}
}

bool Scene::load(const char *fname)
{
	const aiScene *scene; //used to get data from the loaded scene from the file.
    Mesh *mesh = NULL;
    Object *obj = NULL;//used as a temporary object. Used with add_object(Object *obj).
    Light *lt = NULL; //used as a temporary object. Used with add_light(Light *lt).
    Camera *c = NULL;

    scene = aiImportFile(fname, aiProcess_GenSmoothNormals | aiProcess_Triangulate
               | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	
	if(!scene) {
		std::cout<<"ERROR :Failed to load:"<<fname<<std::endl;
		std::cout<<aiGetErrorString()<<std::endl;
        std::cin.get();
		return false;
	}
	else {
		std::cout<<"FILE: "<<fname<<" successfully loaded!!!!"<<std::endl<<std::endl;
	}

	//load all the meshes
	if(scene->HasMeshes()) {

		//for all the meshes of the scene...
        for(unsigned int i = 0 ; i < scene->mNumMeshes; i++) {
			
			aiMesh *ai_m = scene->mMeshes[i];
            
			if((mesh = load_mesh(ai_m))) {
                mesh_by_aimesh[ai_m] = mesh; //map the meshes vector to the assimp mesh data structure.
				add_mesh(mesh);
			}
		}
	}

	//load the nodes recursively
	if(scene->mRootNode) {

        for(unsigned int i = 0 ; i < scene->mRootNode->mNumChildren ; i++) {
            obj = load_node(scene , scene->mRootNode->mChildren[i]);
			if(obj)
				add_object(obj);
		}
	}
	else {
		std::cout<<"ERROR: The file "<<fname<<" has no root node!!"<<std::endl<<std::endl;
		return false;
	}

	//load the lights
	if(scene->HasLights()) {
        for(unsigned int i = 0 ; i < scene->mNumLights ; i++) {
            aiLight *light = scene->mLights[i];
            lt = load_light(light , objects);

            if(lt)
				add_light(lt);
		}
	}

	//load the cameras
	if(scene->HasCameras()) {
        for(unsigned int i = 0 ; i < scene->mNumCameras ; i++) {
            aiCamera *cam = scene->mCameras[i];
            c = load_camera(cam , objects);

            if(c)
				add_camera(c);
		}
		this->set_active_camera();
	}

	aiReleaseImport(scene);
	return true;
}

bool Scene::load_animation(const char *fname , const char *anim_name, bool loop_state)
{
    const aiScene *scene;

    scene = aiImportFile(fname, 0);

    if(!scene) {
        std::cout<<"ERROR :Failed to load:"<<fname<<std::endl;
        std::cout<<aiGetErrorString()<<std::endl;
        std::cin.get();
        return false;
    }

    if(!scene->HasAnimations()) {
        std::cout<<"This scene has no animations!!"<<std::endl;
        return false;
    }

    setup_node_animations(scene , objects[0] , anim_name, loop_state);

    aiReleaseImport(scene);

    return true;
}

void Scene::add_mesh(Mesh *mesh)
{
	meshes.push_back(mesh);
}

void Scene::add_object(Object *obj)
{
	objects.push_back(obj);
}

void Scene::add_light(Light *lt)
{
	lights.push_back(lt);
}

void Scene::add_camera(Camera *cam)
{
	cameras.push_back(cam);
}


Mesh* Scene::get_mesh(unsigned int idx)
{
	return meshes[idx];
}

Object* Scene::get_object(unsigned int idx)
{
	return objects[idx];
}

Light* Scene::get_light(unsigned int idx)
{
	return lights[idx];
}

Camera* Scene::get_camera(unsigned int idx)
{
	return cameras[idx];
}

unsigned int Scene::get_mesh_count() const
{
	return (unsigned int)meshes.size();
}

unsigned int Scene::get_object_count() const
{
	return (unsigned int)objects.size();
}

unsigned int Scene::get_light_count() const
{
    return (unsigned int)lights.size();
}

unsigned int Scene::get_camera_count() const
{
    return (unsigned int)cameras.size();
}

Mesh* Scene::get_mesh(std::string name)
{
	if(!meshes.empty()) {
		for(unsigned int i=0;i<meshes.size();i++) {
			if(meshes[i]->has_name()) {
                std::string mesh_name = meshes[i]->get_name();
                if(mesh_name.compare(name) == 0) {
					return meshes[i];
				}
			}
		}
	}
	std::cout<<"ERROR:: The Scene Object Vector is empty!"<<std::endl;
	return 0;
}


//TODO: Make this search recursively through the children.
Object* Scene::get_object(std::string name)
{
	if (!objects.empty()){
		for (unsigned int i = 0; i < objects.size(); i++){
			Object *obj = NULL;
			obj = lookup_object(objects[i], name);

			if (obj)
				return obj;
		}
	}
	else{
		std::cout << "ERROR:: The Scene Object vector is empty!" << std::endl;
	}

	return NULL;
}

Camera* Scene::get_camera(std::string name)
{
	if(!cameras.empty()) {
        for(unsigned int i = 0 ; i < cameras.size() ; i++) {
            std::string camera_name = cameras[i]->get_name();
            if(camera_name.compare(name) == 0)
				return cameras[i];
		}
	}
	std::cout<<"ERROR:: The Scene Camera Vector is empty!"<<std::endl;
	return NULL;
}

void Scene::setup_lights()
{
    for(unsigned int i = 0 ; i < lights.size() ; i++) {
		lights[i]->set_light();
	}
}

void Scene::setup_cameras(long time)
{
	set_active_camera();
	active_camera->set_camera(time);
}

void Scene::setup_scene(long time)
{
	if((has_cameras()))
		setup_cameras(time);
    if((has_lights()))
        setup_lights();
}

void Scene::set_active_camera()
{
    active_camera = cameras[0];
}

void Scene::render(unsigned int render_mask, long time)
{
    for(unsigned int i = 0 ; i < objects.size() ; i++){
        objects[i]->render(render_mask, time);
	}
}

void Scene::start_animation(unsigned int idx, long start_time)
{
    if(!objects.empty()){
        for(unsigned int i = 0 ; i < objects.size() ; i++){
            objects[i]->start_animation(idx, start_time);
        }
    }
}

int Scene::lookup_animation(const char *name)
{
    return objects[0]->lookup_animation(name);
}

void Scene::set_curr_anim_speed(float speed)
{
    if(!objects.empty()){
        for(unsigned int i = 0 ; i < objects.size() ; i++){
            objects[i]->set_hierarchy_anim_speed(speed);
        }
    }
}

void Scene::set_anim_speed(unsigned int idx, float speed)
{
    if(!objects.empty()){
        for(unsigned int i = 0 ; i < objects.size() ; i++){
            objects[i]->set_hierarchy_anim_speed(idx, speed);
        }
    }
}

long Scene::get_max_anim_time()
{
    if(!objects.empty()){
        for(unsigned int i = 0 ; i < objects.size() ; i++){

            long time = objects[i]->get_max_anim_time();

            if(time > max_anim_time)
                max_anim_time = time;
        }
    }

    return max_anim_time;
}

long Scene::get_max_anim_time(unsigned int idx)
{
    static long max_anim_time = 0;

    if(!objects.empty()){
        for(unsigned int i = 0 ; i < objects.size() ; i++){

            long time = objects[i]->get_max_anim_time(idx);

            if(time > max_anim_time)
                max_anim_time = time;
        }
    }

    return max_anim_time;
}

bool Scene::has_meshes()
{
	if(!meshes.empty())
		return true;

	return false;
}

bool Scene::has_objects()
{
	if(!objects.empty())
		return true;

	return false;
}

bool Scene::has_cameras()
{
	if(!cameras.empty())
		return true;

	return false;
}

bool Scene::has_lights()
{
	if(!lights.empty())
		return true;

	return false;
}

bool Scene::is_anim_finished(long time)
{
    Object *obj = get_object(0);

    if(obj->get_anim_looping_state()) {
        long total_time = get_max_anim_time();
        long diff = (time - obj->get_anim_start_time()) * obj->get_anim_speed();

        if(diff >= total_time)
            return true;
    }

    return false;
}

//----------PRIVATE FUNCTIONS--------------
Object* Scene::lookup_object(Object *obj, const std::string &name)
{
	if (obj->get_name().compare(name) == 0){
		return obj;
	}

	for (unsigned int i = 0; i < obj->get_children_count(); i++){
		Object *res = NULL;
		Object *child = (Object*)obj->get_child(i);
		res = lookup_object(child, name);

		if (res)
			return res;
	}

	return NULL;
}
//-----------------------------------------

//-------STATIC FUNCTIONS------------------

static Mesh *load_mesh(const aiMesh *aimesh)
{
	Vertex tmp_vertex; //used to add vertices to the mesh.
	Face tmp_face; //used to add faces to the mesh.

    Mesh* tmp_mesh = new Mesh;

	tmp_mesh->set_name(aimesh->mName.data);

    for(unsigned int i = 0 ; i < aimesh->mNumVertices ; i++) {

		if(aimesh->mVertices) {
            tmp_vertex.pos.x = aimesh->mVertices[i].x;
            tmp_vertex.pos.y = aimesh->mVertices[i].y;
            tmp_vertex.pos.z = aimesh->mVertices[i].z;
		}
		else {
			std::cout<<"ERROR: The file has no geometry (vertices)."<<std::endl;
            std::cin.get();
		}

		if(aimesh->mNormals) {
            tmp_vertex.normal.x = aimesh->mNormals[i].x;
            tmp_vertex.normal.y = aimesh->mNormals[i].y;
            tmp_vertex.normal.z = aimesh->mNormals[i].z;
		}
		else {
			std::cout<<"ERROR: The mesh has no normals"<<std::endl;
            std::cin.get();
		}

		if(aimesh->mTangents) {
            tmp_vertex.tangent.x = aimesh->mTangents[i].x;
            tmp_vertex.tangent.y = aimesh->mTangents[i].y;
            tmp_vertex.tangent.z = aimesh->mTangents[i].z;
		}
		else {
            tmp_vertex.tangent.x = 1;
            tmp_vertex.tangent.y = tmp_vertex.tangent.z = 0;
		}

		if(aimesh->mTextureCoords[0]) {
            tmp_vertex.u = aimesh->mTextureCoords[0][i].x;
            tmp_vertex.v = aimesh->mTextureCoords[0][i].y;
		}
		else {
            tmp_vertex.u = tmp_vertex.v = 0;
		}

		tmp_mesh->add_vertex(tmp_vertex);
	}

	//get the faces and add them to the mesh.
    for(unsigned int i = 0 ; i < aimesh->mNumFaces ; i++) {

		//j<3 cause we have triangulated the mesh.
        for(unsigned int j = 0 ; j < 3 ; j++) {
            tmp_face.vidx[j] = aimesh->mFaces[i].mIndices[j];
		}
		tmp_mesh->add_index(tmp_face);
	}
	return tmp_mesh;
}

static Object *load_node(const aiScene *sc,const aiNode *n)
{
    Object *o = new Object;
	o->set_name(n->mName.data);

    /*if the node has meshes, load them*/
    if(n->mNumMeshes) {

        if(n->mNumMeshes > 1) {
			std::cout<<"WARNING: The node has more than 1 meshes!!!"<<std::endl;
			std::cout<<"NODE: "<<n->mName.data<<std::endl;
			std::cout<<"NUM MESHES: "<<n->mNumMeshes<<std::endl<<std::endl;
		}

        aiMesh *m = sc->mMeshes[n->mMeshes[0]];
        aiMaterial *mat = sc->mMaterials[m->mMaterialIndex]; //get the material index.
		
		if(mat) {
			o->load_material(mat);
		}
		else {
			std::cout<<"WARNING: Can't load material from the specified file!"<<std::endl;
		}
		o->set_mesh(mesh_by_aimesh[m]);
	}
	else {
		o->set_mesh(NULL);
	}

    /*load the default node transformations*/
    Matrix4x4 mat = ass_mat(n->mTransformation);
    Vector3 translation = mat.get_translation();
    Quaternion rotation = mat.get_rotation_quat();
    Vector3 scaling = mat.get_scaling();
    o->set_position(translation);
    o->set_rotation(rotation);
    o->set_scaling(scaling);

    /*recursion for all the children*/
    for(unsigned int i = 0 ; i < n->mNumChildren ; i++) {
        Object *child = load_node(sc , n->mChildren[i]);
		if(child) {
			o->add_child(child);
		}
	}

    object_by_name[o->get_name()] = o;

	return o;
}

static Light *load_light(const aiLight *l,const std::vector<Object*> objs)
{
    Light *tmp_lt = new Light();
    std::string light_pos = l->mName.data;
	light_pos.append("_$AssimpFbx$_Translation");

    for(unsigned int i = 0 ; i < objs.size() ; i++) {
        
		std::string obj_name = objs[i]->get_name();
        
		if(light_pos.compare(obj_name) == 0) {
			tmp_lt->set_position(objs[i]->get_position());
		}
	}
	
	tmp_lt->set_color_diff(l->mColorDiffuse.r,l->mColorDiffuse.g,l->mColorDiffuse.b);//diffuse light color.
	tmp_lt->set_att_constant(1);//constant attenuation.
	tmp_lt->set_att_linear(0); //linear attenuation.
	tmp_lt->set_att_quadratic(0);//quadratic attenuation.

	if(tmp_lt)
		return tmp_lt;
	else
		return 0;
}

static Camera* load_camera(const aiCamera *cam,const std::vector<Object*> objs)
{
	 //target camera loading
    TargetCamera *tar_cam = new TargetCamera();
    XFormNode *tar = new XFormNode();

    std::string cam_name = cam->mName.data;
    std::string cam_pos = cam_name.append("_$AssimpFbx$_Translation");
    cam_name = cam->mName.data;
    std::string cam_target_pos = cam_name.append(".Target_$AssimpFbx$_Translation");

	for(unsigned int i=0;i<objs.size();i++) {
        
		std::string obj_name = objs[i]->get_name();

        if(cam_pos.compare(obj_name) == 0) {
			tar_cam->set_position(objs[i]->get_position());
		}

        if(cam_target_pos.compare(obj_name) == 0) {
			tar->set_position(objs[i]->get_position());
		}
	}

	tar_cam->set_near_clip_plane(cam->mClipPlaneNear);
	tar_cam->set_far_clip_plane(cam->mClipPlaneFar);
	tar_cam->set_horizontal_fov(cam->mHorizontalFOV);
	tar_cam->set_up_vector(ass_vector(cam->mUp));
	tar_cam->set_target(tar);

	if(tar_cam)
		return tar_cam;
	else
		return 0;
}

static void setup_node_animations(const aiScene *sc, XFormNode *node , const char *anim_name, bool loop_state) // add loop state
{
    //ANIMATION LOADING ------------------------------------------

    for(unsigned int i = 0 ; i < sc->mNumAnimations ; i++) {
        
		aiAnimation *aianim = sc->mAnimations[i];
        aiNodeAnim *ainodeanim = NULL;
        Animation *anim = new Animation(anim_name , loop_state);

        for(unsigned int i = 0 ; i < aianim->mNumChannels ; i++) {
            std::string n = aianim->mChannels[i]->mNodeName.data;
            std::string n2 = node->get_name();
            if(strcmp(n.c_str() , n2.c_str()) == 0) {
                ainodeanim = aianim->mChannels[i];
                break;
            }
        }

        if(ainodeanim) {
			
            anim->clear_all_keyframes();

            //load all position (translation) keyframes
            for(unsigned int i = 0; i < ainodeanim->mNumPositionKeys ; i++) {
                KeyframeVec3 keyframe;
                keyframe.v = ass_vector(ainodeanim->mPositionKeys[i].mValue);
                keyframe.time = ass_time(aianim , ainodeanim->mPositionKeys[i].mTime);

                anim->add_position_keyframe(keyframe);
            }

            // load all rotation keyframes
            for(unsigned int i = 0 ; i < ainodeanim->mNumRotationKeys ; i++) {
                KeyframeQuat keyframe;
                keyframe.r = ass_quat(ainodeanim->mRotationKeys[i].mValue);
                keyframe.time = ass_time(aianim, ainodeanim->mRotationKeys[i].mTime);

                if(keyframe.r.length_sq() < SMALL_NUMBER) {
                    continue;
                }

                keyframe.r.normalize();
                anim->add_rotation_keyframe(keyframe);
            }

            // load all scaling keyframes
            for(unsigned int i = 0 ; i < ainodeanim->mNumScalingKeys ; i++) {
                KeyframeVec3 keyframe;
                keyframe.v = ass_vector(ainodeanim->mScalingKeys[i].mValue);
                keyframe.time = ass_time(aianim , ainodeanim->mScalingKeys[i].mTime);

                anim->add_scaling_keyframe(keyframe);
            }

            node->add_animation(anim);
        }
        else {
            KeyframeVec3 pos , scaling;
            KeyframeQuat rot;

            pos.v = node->get_position();
            rot.r = node->get_rotation();
            scaling.v = node->get_scaling();

            anim->replace_position_keyframe(pos , DEFAULT_VALUES_IDX);
            anim->replace_rotation_keyframe(rot , DEFAULT_VALUES_IDX);
            anim->replace_scaling_keyframe(scaling , DEFAULT_VALUES_IDX);

            node->add_animation(anim);
        }
    }

    for(unsigned int i = 0 ; i < node->get_children_count() ; i++) {
        XFormNode *child = node->get_child(i);
        setup_node_animations(sc , child , anim_name, loop_state);
    }
}

static Vector3 ass_vector(const aiVector3D &v)
{
	return Vector3(v.x,v.y,v.z);
}

static Quaternion ass_quat(const aiQuaternion &q)
{
	return Quaternion(q.w,Vector3(q.x,q.y,q.z));
}

static Matrix4x4 ass_mat(const aiMatrix4x4 &ai_m)
{
	Matrix4x4 res;
    memcpy(res[0] , &ai_m , 16*sizeof(float));
	return res;
}

static long ass_time(const aiAnimation *ai_anim,double ai_time)
{
	double sec;

    if(ai_anim->mTicksPerSecond < 1e-6) //assume time is in frames.
        sec = ai_time / 30.0;
	else
        sec = ai_time / ai_anim->mTicksPerSecond;

    if(ai_time < 0)
        std::cout<<"WARNING: NEGAVITE KEYFRAME TIME!"<<std::endl;
    if(ai_time > INT_MAX)
        std::cout<<"WARNING: VERY BIG KEYFRAME TIME!"<<std::endl;

    return (long)(sec * 1000.0);
}

static void print_hierarchy(const aiNode *node)
{
	 static int lvl;
     static int lvlopen[256];

     for(int i = 0 ; i < lvl; i++) {
        putchar(' ');
        if(lvlopen[i]) {
            putchar(i >= lvl - 1 ? '+' : '|');
        } else {
            putchar(i >= lvl - 1 ? '+' : ' ');
        }
     }
     std::cout<<"- \""<<node->mName.data<<"\""<<std::endl;

     lvlopen[lvl] = 1;

     lvl++;
     for(unsigned int i=0; i<node->mNumChildren; i++) {
        if(i == node->mNumChildren - 1) {
            lvlopen[lvl - 1] = 0;
        }
        print_hierarchy(node->mChildren[i]);
     }
     lvl--;
}

static void delete_object_hierarchy(XFormNode *n)
{
    if(n) {
        for(unsigned int i = 0 ; i < n->get_children_count() ; i++) {
            XFormNode *child = n->get_child(i);
            delete_object_hierarchy(child);
        }

        XFormNode *parent = n->get_parent();

        if(parent)
            parent->remove_child(n);

        if(n->get_node_type() == NODE_PSYS) {
            NucEmitter *em = (NucEmitter*)n;
            em->set_active(false);
        }
        else {
            delete n;
        }
    }
}


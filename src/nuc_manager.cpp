#include "nuc_manager.h"
#include "game.h"

NucManager::NucManager()
{

}

NucManager::~NucManager()
{
    if(!emitters.empty())
    {
        std::list<NucEmitter*>::iterator it = emitters.begin();

        while(it != emitters.end())
        {
            delete *it;
            it = emitters.erase(it);
        }
    }
}

void NucManager::add_emitter(NucEmitter *em)
{
    emitters.push_back(em);
}

void NucManager::delete_emitter(NucEmitter *em)
{
    if(!emitters.empty())
    {
        std::list<NucEmitter*>::iterator it = emitters.begin();

        while(it != emitters.end())
        {
            if(em == *it)
            {
                delete *it;
                it = emitters.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
}

void NucManager::dump_emitters()
{
	if (!emitters.empty()){
		std::list<NucEmitter*>::iterator it = emitters.begin();

		while (it != emitters.end()){
			XFormNode *parent = NULL;
			parent = (*it)->get_parent();

			if (parent){
				parent->remove_child((*it));
			}

			delete (*it);
			it = emitters.erase(it);
		}
	}
}

void NucManager::init_emitters()
{
    if(!emitters.empty())
    {
        std::list<NucEmitter*>::iterator it = emitters.begin();

        while(it != emitters.end())
        {
            (*it)->init();

            it++;
        }
    }
}

void NucManager::render(unsigned int render_mask, long time)
{
    if(!emitters.empty())
    {
        std::list<NucEmitter*>::iterator it = emitters.begin();

        while(it != emitters.end())
        {
            (*it)->render(render_mask, time);

            it++;
        }
    }
}

void NucManager::update(float dt, long time)
{
    if(!emitters.empty()){
        std::list<NucEmitter*>::iterator it = emitters.begin();

        while(it != emitters.end()){
            (*it)->update(dt, time);

            if( !((*it)->is_active()) && (*it)->is_marked_for_death()
                    && ((*it)->get_particle_count() == 0) ){
                XFormNode *parent = NULL;
                parent = (*it)->get_parent();

                if(parent)
                    parent->remove_child(*it);

                delete *it;
                it = emitters.erase(it);
            }
            else{
                it++;
            }
        }
    }
}

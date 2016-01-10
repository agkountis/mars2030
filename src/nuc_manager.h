#ifndef NUC_MANAGER_H
#define NUC_MANAGER_H
#include "nuc_emitter.h"
#include <vector>

class NucManager
{
private:
    std::list<NucEmitter*> emitters;

public:
    NucManager();
    ~NucManager();

    void add_emitter(NucEmitter *em);
    void delete_emitter(NucEmitter *em);
	void dump_emitters();

    void init_emitters();

    void render(unsigned int render_mask, long time = 0);
    void update(float dt, long time = 0);
};

#endif // NUC_MANAGER_H

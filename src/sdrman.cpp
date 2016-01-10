#include <assert.h>
#include "sdrman.h"

ShaderSet *vsdrset, *psdrset;

bool sdrman_init()
{
	if(vsdrset) {
		assert(psdrset);
		return true;	// already initialized
	}
	try {
		vsdrset = new ShaderSet(GL_VERTEX_SHADER);
		psdrset = new ShaderSet(GL_FRAGMENT_SHADER);
	}
	catch(...) {
		delete vsdrset;
		return false;
	}
	return true;
}

void sdrman_destroy()
{
	delete vsdrset;
	delete psdrset;
	vsdrset = psdrset = 0;
}

ShaderProg *get_sdrprog(const char *vname, const char *pname)
{
	if(!vsdrset) {
		sdrman_init();
	}

	Shader *vsdr = vname ? vsdrset->get(vname) : 0;
	Shader *psdr = pname ? psdrset->get(pname) : 0;

	if(vname && !vsdr) {
		return 0;
	}
	if(pname && !psdr) {
		return 0;
	}
	if(!vsdr && !psdr) {
		return 0;
	}

	ShaderProg *prog = new ShaderProg;
	if(!prog->create(vsdr, psdr)) {
		delete prog;
		return 0;
	}
	return prog;
}

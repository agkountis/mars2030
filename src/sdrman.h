#ifndef SDRMAN_H_
#define SDRMAN_H_

#include "shader.h"

extern ShaderSet *vsdrset, *psdrset;

bool sdrman_init();
void sdrman_destroy();

ShaderProg *get_sdrprog(const char *vname, const char *pname);

#endif	// SDRMAN_H_

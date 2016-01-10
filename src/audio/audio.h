#ifndef AUDIO_H_
#define AUDIO_H_

#include "vmath/vmath.h"

bool init_audio();
void destroy_audio();

void set_audio_listener(const Matrix4x4 &xform);

#endif  // AUDIO_H_

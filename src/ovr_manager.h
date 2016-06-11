#ifndef OVR_MANAGER_H_
#define OVR_MANAGER_H_

#ifdef WIN32
#define OVR_OS_WIN32
#include <OVR/OVR_CAPI.h>
#include <OVR/OVR_CAPI_GL.h>
#endif

#include "render_target.h"
#include "vmath.h"

struct OvrData {
	ovrSession session;
	ovrGraphicsLuid luid;
	ovrHmdDesc hmd_desc;
	ovrSizei eyeres[2];
	ovrEyeRenderDesc eye_rdesc[2];
	ovrVector3f hmd_to_eye_view_offset[2];
	ovrLayerEyeFov layer;
	ovrTextureSwapChain tex_swap_chain;
	ovrTextureSwapChainDesc tex_swap_chain_desc;
	//ovrSwapTextureSet *swap_tex_set;
	ovrMirrorTexture mirror_tex;
	ovrMirrorTextureDesc mirror_tex_desc;
	//ovrGLTexture *mirror_tex;
	ovrViewScaleDesc view_scale_desc;
};

struct Vector2i {
	int x;
	int y;
};

struct Viewport {
	Vector2i position;
	Vector2i size;
};

struct OvrTransformationData {
	Matrix4x4 view;
	Matrix4x4 projection;
	Viewport viewport;
};

struct OvrEyePoses {
	ovrPosef pose[2];
};

class OvrManager {
private:
	OvrData ovr_data;
	RenderTarget ovr_rtarg;
	OvrTransformationData ovr_transformation_data;
	OvrEyePoses eye_poses;

public:
	bool init_ovr_library();
	void shutdown_ovr_library();

	bool init_ovr_data(int win_width, int win_height);
	void destroy_ovr_data();

	void calculate_eye_poses();
	const OvrTransformationData &get_ovr_transformation_data_per_eye(unsigned int eye);

	void begin_ovr_frame();
	bool submit_ovr_frame();
	void draw_ovr_mirror_texture(int win_width, int win_height);

	RenderTarget &get_ovr_render_target();
};

#endif
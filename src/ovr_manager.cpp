#include <iostream>
#include <algorithm>
#include "ovr_manager.h"
#include "opengl.h"


bool OvrManager::init_ovr_library() 
{
	if (OVR_FAILURE(ovr_Initialize(nullptr))) {
		std::cerr << "Failed to Initialize the OVR Library!!!" << std::endl;
		return false;
	}

	return true;
}

void OvrManager::shutdown_ovr_library()
{
	ovr_Shutdown();
}

bool OvrManager::init_ovr_data(int win_width, int win_height)
{
	if (OVR_FAILURE(ovr_Create(&ovr_data.session, &ovr_data.luid))){
		std::cerr << "Failed to open the Oculus HMD!! If u want a virtual HMD enable it through the Rift Config Util!" << std::endl;
		return false;
	}

	ovr_data.hmd_desc = ovr_GetHmdDesc(ovr_data.session);
	std::cout << "Initialized HMD: " << ovr_data.hmd_desc.Manufacturer << " - " << ovr_data.hmd_desc.ProductName << std::endl;

	/*The new SDK has all the tracking capabilities enabled by default.*/

	ovr_data.eye_rdesc[0] = ovr_GetRenderDesc(ovr_data.session, ovrEye_Left, ovr_data.hmd_desc.DefaultEyeFov[0]);
	ovr_data.eye_rdesc[1] = ovr_GetRenderDesc(ovr_data.session, ovrEye_Right, ovr_data.hmd_desc.DefaultEyeFov[1]);

	ovr_data.hmd_to_eye_view_offset[0] = ovr_data.eye_rdesc[0].HmdToEyeViewOffset;
	ovr_data.hmd_to_eye_view_offset[1] = ovr_data.eye_rdesc[1].HmdToEyeViewOffset;

	ovr_data.view_scale_desc.HmdSpaceToWorldScaleInMeters = 1.0;
	ovr_data.view_scale_desc.HmdToEyeViewOffset[0] = ovr_data.hmd_to_eye_view_offset[0];
	ovr_data.view_scale_desc.HmdToEyeViewOffset[1] = ovr_data.hmd_to_eye_view_offset[1];

	ovr_data.layer.Header.Type = ovrLayerType_EyeFov;
	ovr_data.layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

	ovr_data.layer.Fov[0] = ovr_data.eye_rdesc[0].Fov;
	ovr_data.layer.Fov[1] = ovr_data.eye_rdesc[1].Fov;

	/*Retrieve the optimal render target resolution for each eye*/
	ovr_data.eyeres[0] = ovr_GetFovTextureSize(ovr_data.session, ovrEye_Left, ovr_data.hmd_desc.DefaultEyeFov[0], 1.0);
	ovr_data.eyeres[1] = ovr_GetFovTextureSize(ovr_data.session, ovrEye_Right, ovr_data.hmd_desc.DefaultEyeFov[1], 1.0);

	/*Create a single render target texture to encompass both eyes.*/
	ovr_rtarg.set_fb_width(ovr_data.eyeres[0].w + ovr_data.eyeres[1].w);

	ovr_rtarg.set_fb_height(std::max(ovr_data.eyeres[0].h, ovr_data.eyeres[1].h));

	ovrResult res = ovr_CreateSwapTextureSetGL(ovr_data.session, GL_SRGB8_ALPHA8, ovr_rtarg.get_fb_width(), ovr_rtarg.get_fb_height(), &ovr_data.swap_tex_set);
	if (OVR_FAILURE(res)) {
		std::cout << "Failed to allocate the ovrTextureSet!" << std::endl;
		return false;
	}

	ovr_rtarg.set_shad_tex_width(2048);
	ovr_rtarg.set_shad_tex_height(2048);

	/*Assign a texture from the texture set so that the render target won't allocate a new one by itself during creation*/
	ovr_rtarg.set_color_attachment(((ovrGLTexture*)&ovr_data.swap_tex_set->Textures[0])->OGL.TexId);
	ovr_rtarg.create(RT_ALL);

	res = ovr_CreateMirrorTextureGL(ovr_data.session, GL_SRGB8_ALPHA8, win_width, win_height, reinterpret_cast<ovrTexture**>(&ovr_data.mirror_tex));
	if (OVR_FAILURE(res)) {
		std::cerr << "Failed to allocate the ovr Mirror Texture!" << std::endl;
		return false;
	}

	ovr_data.layer.ColorTexture[0] = ovr_data.swap_tex_set;
	ovr_data.layer.ColorTexture[1] = ovr_data.swap_tex_set;

	ovrRecti rect0 = {
			{ 0, 0 },
			{ ovr_rtarg.get_fb_width() / 2, ovr_rtarg.get_fb_height() }
	};
	ovrRecti rect1 = {
			{ ovr_rtarg.get_fb_width() / 2, 0 },
			{ ovr_rtarg.get_fb_width() / 2, ovr_rtarg.get_fb_height() }
	};

	ovr_data.layer.Viewport[0] = rect0;
	ovr_data.layer.Viewport[1] = rect1;

	return true;
}

void OvrManager::destroy_ovr_data()
{
	ovr_DestroySwapTextureSet(ovr_data.session, ovr_data.swap_tex_set);
	ovr_DestroyMirrorTexture(ovr_data.session, reinterpret_cast<ovrTexture*>(ovr_data.mirror_tex));
	ovr_Destroy(ovr_data.session);
}

void OvrManager::calculate_eye_poses()
{
	double display_midpoint_seconds = ovr_GetPredictedDisplayTime(ovr_data.session, 0);
	double sensor_time = ovr_GetTimeInSeconds();
	ovr_data.layer.SensorSampleTime = sensor_time;
	ovrTrackingState hmd_state = ovr_GetTrackingState(ovr_data.session, display_midpoint_seconds, ovrTrue);

	ovr_CalcEyePoses(hmd_state.HeadPose.ThePose, ovr_data.hmd_to_eye_view_offset, eye_poses.pose);
}

const OvrTransformationData& OvrManager::get_ovr_transformation_data_per_eye(unsigned int eye)
{
	/*Fill up the Viewport structure*/
	ovr_transformation_data.viewport.position.x = ovr_data.layer.Viewport[eye].Pos.x;
	ovr_transformation_data.viewport.position.y = ovr_data.layer.Viewport[eye].Pos.y;

	ovr_transformation_data.viewport.size.x = ovr_data.layer.Viewport[eye].Size.w;
	ovr_transformation_data.viewport.size.y = ovr_data.layer.Viewport[eye].Size.h;
	/*-------------------------------------------------------------------------*/

	/*Calculate the Projection Matrix*/
	ovr_transformation_data.projection.reset_identity();

	ovrMatrix4f proj = ovrMatrix4f_Projection(ovr_data.layer.Fov[eye], 0.01, 40000.0, ovrProjection_RightHanded);
	memcpy(ovr_transformation_data.projection[0], proj.M, 16 * sizeof(float));
	/*-------------------------------------------------------------------------*/

	/*Calculate the View Matrix*/
	ovr_transformation_data.view.reset_identity();

	Quaternion q = Quaternion(eye_poses.pose[eye].Orientation.w, eye_poses.pose[eye].Orientation.x,
		eye_poses.pose[eye].Orientation.y, eye_poses.pose[eye].Orientation.z);
	ovr_transformation_data.view.rotate(q.inverse().normalized());

	ovr_transformation_data.view.translate(Vector3(-eye_poses.pose[eye].Position.x, -eye_poses.pose[eye].Position.y, -eye_poses.pose[eye].Position.z));
	/*-------------------------------------------------------------------------*/

	return ovr_transformation_data;
}

void OvrManager::begin_ovr_frame()
{
	ovrGLTexture *tex;

	/*Increment the texture set index so that we use the next texture before writing*/
	ovr_data.swap_tex_set->CurrentIndex = (ovr_data.swap_tex_set->CurrentIndex + 1)
		% ovr_data.swap_tex_set->TextureCount;

	tex = (ovrGLTexture*)&ovr_data.swap_tex_set->Textures[ovr_data.swap_tex_set->CurrentIndex];
	ovr_rtarg.set_and_assign_color_attachment(tex->OGL.TexId);

	/*Start drawing onto our texture render target.*/
	ovr_rtarg.bind(RT_COLOR_AND_DEPTH);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool OvrManager::submit_ovr_frame()
{
	ovr_rtarg.unbind();

	ovr_data.layer.RenderPose[0] = eye_poses.pose[0];
	ovr_data.layer.RenderPose[1] = eye_poses.pose[1];

	ovrLayerHeader *layer_header = &ovr_data.layer.Header;

	ovrResult result = ovr_SubmitFrame(ovr_data.session, 0, &ovr_data.view_scale_desc, &layer_header, 1);

	return result == ovrSuccess;
}
void OvrManager::draw_ovr_mirror_texture(int win_width, int win_height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	unsigned int t = ovr_data.mirror_tex->OGL.TexId;
	glUseProgram(0);
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, t);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, win_width, win_height);

	/*Use top left as texture coordinate origin for the mirror texture in order to cancel the mirroring effect*/
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex2f(-1, -1);
	glTexCoord2f(1, 1);
	glVertex2f(1, -1);
	glTexCoord2f(1, 0);
	glVertex2f(1, 1);
	glTexCoord2f(0, 0);
	glVertex2f(-1, 1);
	glEnd();
	glPopAttrib();
}

RenderTarget& OvrManager::get_ovr_render_target()
{
	return ovr_rtarg;
}
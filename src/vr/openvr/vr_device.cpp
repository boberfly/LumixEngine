#include "vr_device.h"
#include "engine/log.h"
#include "engine/engine.h"
#include "engine/iplugin.h"


namespace Lumix
{

Matrix convertOpenVRMatrix34(vr::HmdMatrix34_t mat)
{
	return Matrix(mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0f,
		mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0f,
		mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0f,
		mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f);
}

struct VRDeviceImpl : public VRDevice
{

	Engine* m_engine;
	Array<TrackedDevice> m_tracked_devices;
	u32 m_hmdIndex;
	u32 m_left_controller_index;
	u32 m_right_controller_index;

	vr::IVRSystem* m_system;
	vr::IVRCompositor* m_compositor;
	vr::IVRChaperone* m_chaperone;

	VRDeviceImpl()
		: m_engine(nullptr)
		, m_tracked_devices(engine.getAllocator())
		, m_hmd_index(0)
		, m_left_controller_index(0)
		, m_right_controller_index(0)
		, m_system(nullptr)
		, m_compositor(nullptr)
		, m_chaperone(nullptr)
	{}


	bool init(Engine& engine)
	{
		m_engine = &engine;
		m_system = (vr::IVRSystem*)engine.getPlatformData().session;
		m_compositor = (vr::IVRCompositor*)engine.getPlatformData().compositor;

		if (m_system == nullptr)
		{
			g_log_error.log("VR") << "OpenVR System interface doesn\'t exist.";
			ASSERT(false);
			return false;
		}
		if (m_compositor == nullptr)
		{
			g_log_error.log("VR") << "OpenVR Compositor interface doesn\'t exist.";
			ASSERT(false);
			return false;
		}

		//m_chaperone = (vr::IVRChaperone *)vr::VR_GetGenericInterface(vr::IVRChaperone_Version, &hmdError);

		m_tracked_devices.reserve(vr::k_unMaxTrackedDeviceCount);

		for(u32 i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
		{
			vr::TrackedDeviceClass cur_device = m_system->GetTrackedDeviceClass(i);
			if (cur_device == vr::TrackedDeviceClass_HMD)
			{
				m_hmd_index = i;
			}
			else if (cur_device == vr::TrackedDeviceClass_Controller)
			{
				vr::ETrackedControllerRole role;
				role = GetControllerRoleForTrackedDeviceIndex(i);
				if (role == TrackedControllerRole_LeftHand)
				{
					m_left_controller_index = i;
				}
				else if (role == TrackedControllerRole_RightHand)
				{
					m_right_controller_index = i;
				}
			}
			else if (cur_device == vr::TrackedDeviceClass_TrackingReference)
			{
				continue; // Need to get the light houses...
			}
		}

		return true;
	}


	~VRDeviceImpl()
	{
		m_trackedDevices.clear();
	}

	void refreshTracking() override 
    {
		if(!m_system || !m_compositor) return;

		// Query the compositor to return current positions of all tracked devices
		vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
        m_compositor->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

		// Now that we have the poses for all of our devices, cycle through them to update their state
		for(u32 i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
		{
			const auto& pose = poses[i];
			m_trackedDevices[i].absoluteTracking = convertOpenVRMatrix34(pose.absoluteTracking);
			m_trackedDevices[i].velocity = Vec3(pose.velocity);
			m_trackedDevices[i].angularVeolcity = Vec3(pose.angularVelocity);
			m_trackedDevices[i].poseIsValid = pose.poseIsValid;
			m_trackedDevices[i].deviceIsConnected = pose.deviceIsConnected;
        }

    }

	TrackedDevice& getHMDTracking() override
	{
		return m_trackedDevices[m_hmdIndex];
	}

	TrackedDevice& getRightControllerTracking() override
	{
		return m_trackedDevices[m_rightControllerIndex];
	}

	TrackedDevice& getLeftControllerTracking() override
	{
		return m_trackedDevices[m_leftControllerIndex];
	}

};


static NullVRDevice g_null_device;


VRDevice* VRDevice::create(Engine& engine)
{
	auto* device = LUMIX_NEW(engine.getAllocator(), VRDeviceImpl);
	if (!device->init(engine))
	{
		LUMIX_DELETE(engine.getAllocator(), device);
		g_log_warning.log("VR") << "Using null device";
		return &g_null_device;
	}
	return device;
}


void VRDevice::destroy(VrDevice& device)
{
	if (&device == &g_null_device) return;
	LUMIX_DELETE(static_cast<VRDeviceImpl&>(device).m_engine->getAllocator(), &device);
}


} // namespace Lumix
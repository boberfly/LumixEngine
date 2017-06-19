#include "vr_scene.h"
#include "vr_device.h"
#include "vr_system.h"
#include "engine/blob.h"
#include "engine/crc32.h"
#include "engine/engine.h"
#include "engine/iallocator.h"
#include "engine/lua_wrapper.h"
#include "engine/matrix.h"
#include "engine/property_register.h"
#include "engine/resource_manager.h"
#include "engine/resource_manager_base.h"
#include "engine/serializer.h"
#include "engine/universe/universe.h"
#include "lua_script/lua_script_system.h"


namespace Lumix
{


static const ComponentType HMD_TYPE = PropertyRegister::getComponentType("hmd");
static const ComponentType SCENE_TRACKED_DEVICE_TYPE = PropertyRegister::getComponentType("scene_tracked_device");


struct HMD
{
	Entity entity;
};


struct SceneTrackedDevice
{
	Entity entity;
	int tracked_device_index;
};


struct VRSceneImpl LUMIX_FINAL : public VRScene
{
	VRSceneImpl(VRSystem& system, Universe& context, IAllocator& allocator)
		: m_allocator(allocator)
		, m_universe(context)
		, m_system(system)
		, m_device(system.getDevice()
		, m_scene_trackedDevices(allocator))
	{
		m_hmd.entity = INVALID_ENTITY;
		context.registerComponentType(HMD_TYPE, this, &VRSceneImpl::serializeHMD, &VRSceneImpl::deserializeHMD);
		context.registerComponentType(SCENE_TRACKED_DEVICE_TYPE, this, &VRSceneImpl::serializeSceneTrackedDevice, &VRSceneImpl::deserializeSceneTrackedDevice);
	}


	void serializeHMD(ISerializer&, ComponentHandle) {}


	void deserializeHMD(IDeserializer&, Entity entity, int /*scene_version*/)
	{
		m_hmd.entity = entity;
		m_universe.addComponent(entity, HMD_TYPE, this, {0});
	}


	void serializeSceneTrackedDevice(ISerializer&, ComponentHandle) {}


	void deserializeSceneTrackedDevice(IDeserializer&, Entity entity, int /*scene_version*/)
	{
		m_hmd.entity = entity;
		m_universe.addComponent(entity, SCENE_TRACKED_DEVICE_TYPE, this, {0});
	}


	void update(float time_delta, bool paused) override
	{
		if (!m_is_game_running || paused) return;

		PROFILE_FUNCTION();

		m_device->refreshTracking();

		auto tracked_pose_hmd = m_device->getHMDTracking();
		auto tracked_pose_right_controller = m_device->getRightControllerTracking();
		auto tracked_pose_left_controller = m_device->getLeftControllerTracking();
		//auto tracked_pose_lighthouse_one = m_device->getLightHouseOneTracking();
		//auto tracked_pose_lighthouse_two = m_device->getLightHouseTwoTracking();

		for (auto device : m_scene_tracked_devices)
		{
			switch ((TrackedDeviceType)index)
			{
				case TrackedDeviceType::HMD:
					m_universe.setMatrix(device.entity, tracked_pose_hmd.absoluteTracking);
				case TrackedDeviceType::RIGHT_CONTROLLER:
					m_universe.setMatrix(device.entity, tracked_pose_right_controller.absolute_tracking);
				case TrackedDeviceType::LEFT_CONTROLLER:
					m_universe.setMatrix(device.entity, tracked_pose_left_controller.absolute_tracking);
				//case TrackedDeviceType::LIGHTHOUSE_ONE:
				//	m_universe.setMatrix(device.entity, tracked_pose_lightbox_one.absolute_tracking);
				//case TrackedDeviceType::LIGHTHOUSE_TWO:
				//	m_universe.setMatrix(device.entity, tracked_pose_lightbox_two.absolute_tracking);
				default: ASSERT(false);
			}
		}
	}


	void startGame() override
	{
	}


	void stopGame() override
	{
	}


	int getSceneTrackedDeviceCount() const override { return 3; } // TODO


	const char* getSceneTrackedDeviceName(int index) override // TODO
	{
		switch ((TrackedDeviceType)index)
		{
			case TrackedDeviceType::HMD: return "HMD";
			case TrackedDeviceType::RIGHT_CONTROLLER: return "Right Controller";
			case TrackedDeviceType::LEFT_CONTROLLER: return "Left Controller";
			//case TrackedDeviceType::LIGHTHOUSE_ONE: return "LightHouse One";
			//case TrackedDeviceType::LIGHTHOUSE_TWO: return "LightHouse Two";
			default: ASSERT(false); return "Unknown";
		}
	}


	int getSceneTrackedDeviceIndex(ComponentHandle cmp) override
	{
		return m_scene_tracked_devices[{cmp.index}].tracked_device_index;
	}


	void setSceneTrackedDeviceIndex(ComponentHandle cmp, int index) override
	{
		m_scene_tracked_devices[{cmp.index}].tracked_device_index = index;
	}


	ComponentHandle createHMD(Entity entity)
	{
		if (m_hmd.entity != INVALID_ENTITY)
		{
			g_log_warning.log("VR") << "HMD already exists";
			return INVALID_COMPONENT;
		}

		m_hmd.entity = entity;
		m_universe.addComponent(entity, HMD_TYPE, this, {0});
		return {0};
	}


	ComponentHandle createSceneTrackedDevice(Entity entity)
	{
		if (m_hmd.entity != INVALID_ENTITY)
		{
			g_log_warning.log("VR") << "Scene Tracked Device already exists";
			return INVALID_COMPONENT;
		}

		m_hmd.entity = entity;
		m_universe.addComponent(entity, SCENE_TRACKED_DEVICE_TYPE, this, {0});
		return {0};
	}


	ComponentHandle createComponent(ComponentType type, Entity entity) override;


	void destroyHMD(ComponentHandle component)
	{
		ASSERT(component.index == 0);
		auto entity = m_hmd.entity;
		m_hmd.entity = INVALID_ENTITY;
		m_universe.destroyComponent(entity, HMD_TYPE, this, component);
	}


	void destroySceneTrackedDevice(ComponentHandle component)
	{
		ASSERT(component.index == 0);
		auto entity = m_hmd.entity;
		m_hmd.entity = INVALID_ENTITY;
		m_universe.destroyComponent(entity, SCENE_TRACKED_DEVICE_TYPE, this, component);
	}


	void destroyComponent(ComponentHandle component, ComponentType type) override;


	void deserialize(InputBlob& serializer) override
	{
		clear();

		serializer.read(m_hmd.entity);
		if (m_hmd.entity != INVALID_ENTITY)
		{
			m_universe.addComponent(m_hmd.entity, HMD_TYPE, this, {0});
		}
	}


	ComponentHandle getComponent(Entity entity, ComponentType type) override
	{
		if (type == HMD_TYPE)
		{
			ComponentHandle hmd_cmp = { 0 };
			return m_hmd.entity == entity ? hmd_cmp : INVALID_COMPONENT;
		}
		if (type == SCENE_TRACKED_DEVICE_TYPE)
		{
			int idx = m_scene_tracked_devices.find(entity);
			if (idx < 0) return INVALID_COMPONENT;
			return {entity.index};
		}

		return INVALID_COMPONENT;
	}


	Universe& getUniverse() override { return m_universe; }
	IPlugin& getPlugin() const override { return m_system; }

	AssociativeArray<Entity, SceneTrackedDevice> m_scene_tracked_devices;
	HMD m_hmd;
	IAllocator& m_allocator;
	Universe& m_universe;
	VRSystem& m_system;

};


static struct
{
	ComponentType type;
	ComponentHandle(VRSceneImpl::*creator)(Entity);
	void (VRSceneImpl::*destroyer)(ComponentHandle);
} COMPONENT_INFOS[] = {
	{ HMD_TYPE, &VRSceneImpl::createHMD, &VRSceneImpl::destroyHMD }
};


ComponentHandle VRSceneImpl::createComponent(ComponentType type, Entity entity)
{
	for(auto& i : COMPONENT_INFOS)
	{
		if(i.type == type)
		{
			return (this->*i.creator)(entity);
		}
	}

	return INVALID_COMPONENT;
}


void VRSceneImpl::destroyComponent(ComponentHandle component, ComponentType type)
{
	for(auto& i : COMPONENT_INFOS)
	{
		if(i.type == type)
		{
			(this->*i.destroyer)(component);
			return;
		}
	}
}


VRScene* VRScene::createInstance(VRSystem& system,
	Universe& universe,
	IAllocator& allocator)
{
	return LUMIX_NEW(allocator, VRSceneImpl)(system, universe, allocator);
}


void VRScene::destroyInstance(VRScene* scene)
{
	LUMIX_DELETE(static_cast<VRSceneImpl*>(scene)->m_allocator, scene);
}


} // namespace Lumix
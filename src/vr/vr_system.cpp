#include "vr_system.h"
#include "vr_device.h"
#include "vr_scene.h"
#include "editor/asset_browser.h"
#include "editor/studio_app.h"
#include "editor/utils.h"
#include "editor/world_editor.h"
#include "engine/crc32.h"
#include "engine/engine.h"
#include "engine/iplugin.h"
#include "engine/path.h"
#include "engine/plugin_manager.h"
#include "engine/property_descriptor.h"
#include "engine/property_register.h"
#include "renderer/render_scene.h"


namespace Lumix
{


static void registerProperties(IAllocator& allocator)
{
	PropertyRegister::add("scene_tracked_device",
		LUMIX_NEW(allocator, DynamicEnumPropertyDescriptor<VRScene>)("Device",
			&VRScene::getSceneTrackedDeviceIndex,
			&VRScene::setSceneTrackedDeviceIndex,
			&VRScene::getSceneTrackedDeviceCount,
			&VRScene::getSceneTrackedDeviceName));
}


struct VRSystemImpl LUMIX_FINAL : public VRSystem
{
	explicit VRSystemImpl(Engine& engine)
		: m_engine(engine)
		, m_manager(engine.getAllocator())
		, m_device(nullptr)
	{
		registerProperties(engine.getAllocator());
		//VRScene::registerLuaAPI(m_engine.getState());
		m_device = VRDevice::create(m_engine);
	}


	~VRSystemImpl()
	{
		VRDevice::destroy(*m_device);
		m_manager.destroy();
	}


	Engine& getEngine() override { return m_engine; }
	VRDevice& getDevice() override { return *m_device; }


	const char* getName() const override { return "vr"; }


	void refreshTracking()
	{

	}


	void createScenes(Universe& ctx) override
	{
		auto* scene = VRScene::createInstance(*this, ctx, m_engine.getAllocator());
		ctx.addScene(scene);
	}


	void destroyScene(IScene* scene) override { VRScene::destroyInstance(static_cast<VRScene*>(scene)); }


	Engine& m_engine;
	VRDevice* m_device;
};


LUMIX_PLUGIN_ENTRY(vr)
{
	return LUMIX_NEW(engine.getAllocator(), VRSystemImpl)(engine);
}


} // namespace Lumix


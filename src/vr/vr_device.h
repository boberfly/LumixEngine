#pragma once


#include "engine/lumix.h"


#ifdef STATIC_PLUGINS
	#define LUMIX_VR_API
#elif defined BUILDING_VR
	#define LUMIX_VR_API LUMIX_LIBRARY_EXPORT
#else
	#define LUMIX_VR_API LUMIX_LIBRARY_IMPORT
#endif


namespace Lumix
{


class Engine;
struct IAllocator;
class Path;


struct TrackedDevice
{
	Matrix absoluteTracking;
	Vec3 velocity;
	Vec3 angularVeolcity;
	int32_t trackingResult;
	bool poseIsValid;
	bool deviceIsConnected;
};


class LUMIX_VR_API VRDevice
{
public:
	virtual ~VRDevice() {}

	static VRDevice* create(Engine& engine);
	static void destroy(VRDevice& device);

	virtual void refreshTracking() = 0;

	TrackedDevice& getHMDTracking() = 0;
	TrackedDevice& getRightControllerTracking() = 0;
	TrackedDevice& getLeftControllerTracking() = 0;
};


} // namespace Lumix
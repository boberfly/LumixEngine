#pragma once


#include "engine/iplugin.h"


namespace Lumix
{


class VRSystem;


class VRScene : public IScene
{

	enum class TrackedDeviceType : int
	{
		HMD,
		RIGHT_CONTROLLER,
		LEFT_CONTROLLER,
		//LIGHTHOUSE_ONE,
		//LIGHTHOUSE_TWO
	};

public:
	static VRScene* createInstance(VRSystem& system,
		Universe& universe,
		struct IAllocator& allocator);
	static void destroyInstance(VRScene* scene);
};


} // namespace Lumix
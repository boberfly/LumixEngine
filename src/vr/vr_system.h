#pragma once


#include "engine/iplugin.h"


namespace Lumix
{


class VRDevice;


class VRSystem : public IPlugin
{
	public:
		virtual VRDevice& getDevice() = 0;
		virtual Engine& getEngine() = 0;
};


} // namespace Lumix
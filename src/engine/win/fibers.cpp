#include "engine/fibers.h"
#include <Windows.h>


namespace Lumix
{


namespace Fiber
{


Handle createFromThread(void* parameter)
{
	return ConvertThreadToFiber(parameter);
}


Handle create(int stack_size, FiberProc proc, void* parameter)
{
	return CreateFiber(stack_size, proc, parameter);
}


void destroy(Handle fiber)
{
	DeleteFiber(fiber);
}


void switchTo(Handle fiber)
{
	SwitchToFiber(fiber);
}


void* getParameter()
{
	return GetFiberData();
}


} // namespace Fibers


} // namespace Lumix
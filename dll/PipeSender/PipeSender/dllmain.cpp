// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <ryulib/debug_tools.hpp>
#include <ryulib/PipeSender.hpp>

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

typedef void (*CALLBACK_NOTIFY) (const void* context);

class PipeSenderHandle {
public:
	PipeSender* object; 

	PipeSenderHandle(void* context)
		: context_(context)
	{
		object = new PipeSender();
		object->setOnConnected([&](const void* sender) {
			if (callback_on_connected_ != nullptr) callback_on_connected_(context_);
		});
	}

	~PipeSenderHandle()
	{
		delete object;
	}

	void setOnConnected(CALLBACK_NOTIFY event) { callback_on_connected_ = event; }

private:
	void* context_;
	CALLBACK_NOTIFY callback_on_connected_ = nullptr;
};

extern "C" __declspec(dllexport) PipeSenderHandle* create_PipeSender_object(void* context)
{
	return new PipeSenderHandle(context);
}

extern "C" __declspec(dllexport) void destroy_PipeSender_object(PipeSenderHandle* handle)
{
	delete handle;
}

extern "C" __declspec(dllexport) void PipeSender_setOnConnected(PipeSenderHandle* handle, CALLBACK_NOTIFY event)
{
	handle->setOnConnected(event);
}

extern "C" __declspec(dllexport) bool PipeSender_open(PipeSenderHandle* handle, const char* name)
{
	DebugOutput::trace("PipeSender_open - %s", name);

	return handle->object->open(string(name));
}

extern "C" __declspec(dllexport) void PipeSender_close(PipeSenderHandle* handle)
{
	handle->object->close();
}

extern "C" __declspec(dllexport) void PipeSender_send(PipeSenderHandle* handle, const void* data, int size)
{
	handle->object->send(data, size);
}
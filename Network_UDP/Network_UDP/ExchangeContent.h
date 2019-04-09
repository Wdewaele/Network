#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <chrono>
#include <sstream>

#include <WS2tcpip.h>
#include <minwinbase.h>

typedef void(__stdcall * DebugCallback) (const char * str);
extern DebugCallback gDebugCallback;

extern "C" inline __declspec(dllexport) void RegisterDebugCallback(DebugCallback callback)
{
	if (callback)
	{
		gDebugCallback = callback;
	}
}

enum class State : uint8_t
{
	DISCONNECTED,
	CONNECTING,
	CONNECTED
};

enum class PacketType : uint8_t
{
	CONNECTION,
	RETURN_CONNECTION,
	DATA
};
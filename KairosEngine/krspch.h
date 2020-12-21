#pragma once


#include <Windows.h>
#include <shellapi.h>
#define WIN32_LEAN_AND_MEAN
#include <wrl.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#pragma warning(disable:4265)
#pragma warning(default:4265)

using namespace Microsoft::WRL;

#include "Engine/Core/EngineCore.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseCodes.h"
#include "Engine/Core/BaseTypes.h"
#include "Engine/Core/EngineMath.h"
#include "Engine/Graphics/Core/GraphicsTypes.h"
#include "Engine/Core/Log.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include <SimpleMath.h>
#include "d3dx12/d3dx12.h"

#pragma comment (lib, "d3d12.lib") 
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <chrono>
#include <optional>
#include <functional>
#include <vector>
#include <queue>
#include <map>
#include <deque>
#include <set>
#include <iostream>
#include <mutex>



struct A {
	int i;
};
#pragma once

#include "Core/BaseTypes.h"
#include "Core/EngineCore.h"
#include "Core/Log.h"
#include "Core/EngineMath.h"

//
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include "d3dx12.h"
#include <DirectXTK/SimpleMath.h>

#pragma comment (lib, "d3d12.lib") 
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
//
using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;


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
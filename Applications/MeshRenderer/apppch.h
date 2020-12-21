#pragma once


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

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include "d3dx12/d3dx12.h"

#pragma comment (lib, "d3d12.lib") 
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")



template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T>
using WeakRef = std::weak_ptr<T>;
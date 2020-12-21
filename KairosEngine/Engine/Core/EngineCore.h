#pragma once
#include <memory>
#define KRS_EXPAND_VARGS(x) x
#define BIT(x) (1 << x) // bit shift by 'x' spaces
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
#define INITIALIZER(x) = x;

// macro for creating enums (in case we want to change in the future) 
#define KRS_TYPED_ENUM(EnumName, EnumType) enum EnumName : EnumType
#define KRS_UNSCOPED_ENUM(EnumName) enum : EnumName
#define KRS_DERIVE(TypeName) : public TypeName

#define KRS_BEGIN_NAMESPACE(Name) \
	namespace Name				  \
	{

#define KRS_END_NAMESPACE }

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

// Macros for common sizes
#define _KB(x) (x * 1024)
#define _MB(x) (x * 1024 * 1024)

#define _64KB _KB(64)
#define _1MB _MB(1)
#define _2MB _MB(2)
#define _4MB _MB(4)
#define _8MB _MB(8)
#define _16MB _MB(16)
#define _32MB _MB(32)
#define _64MB _MB(64)
#define _128MB _MB(128)
#define _256MB _MB(256)


const static bool FULL_SCREEN = false;
const static unsigned int WINDOW_WIDTH = 1280;
const static unsigned int WINDOW_HEIGHT = 1024;

// typedefs for smart pointers
namespace Kairos {
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
}



// ASSERTION STUFF
#ifdef KRS_DEBUG
	#if defined(KRS_PLATFORM_WINDOWS)
		#define KRS_DEBUGBREAK() __debugbreak()
	#endif
	#define KRS_ENABLE_ASSERTS
#else 
	#define KRS_DEBUGBREAK()
#endif


#ifdef KRS_ENABLE_ASSERTS
	#define KRS_ASSERT(x, ...) { if(!(x)) { KRS_ERROR("Assertion Failed: {0}", __VA_ARGS__); KRS_DEBUGBREAK(); } }
	#define KRS_CORE_ASSERT(x, ...) { if(!(x)) { KRS_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); KRS_DEBUGBREAK(); } }
#else
	#define KRS_ASSERT(x, ...)
	#define KRS_CORE_ASSERT(x, ...)
#endif
#pragma once
#include <memory>
#include "Log.h"
#define KRS_EXPAND_VARGS(x) x
#define BIT(x) (1 << x) // bit shift by 'x' spaces
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
#define INITIALIZER(x) = x;

// macro for creating enums (in case we want to change in the future) 
#define KRS_TYPED_ENUM(EnumName, EnumType) enum EnumName : EnumType
#define KRS_TYPED_CLASS_ENUM(EnumName, EnumType) enum class EnumName : EnumType
#define KRS_UNSCOPED_ENUM(EnumName) enum : EnumName
#define KRS_DERIVE(TypeName) : public TypeName


#define SAFE_DELETE(p) if ((p)) { delete(p); (p) = nullptr; }

#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

#define ENUM_FLAG_OPERATORS(e_) inline e_ operator& (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b));}  \
    inline e_ operator| (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b));} \
    inline e_& operator|= (e_& a, e_ b){a = a | b; return a;};  \
    inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; };   \
    inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a));}   \
    inline bool is_any_set(e_ val, e_ flag) { return (val & flag) != (e_)0;} \
    inline bool is_all_set(e_ val, e_ flag) { return (val & flag) == flag;}   


#define UNUSED(X) (void*)&X;


#define KRS_BEGIN_NAMESPACE(Name) \
	namespace Name				  \
	{

#define KRS_END_NAMESPACE }

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

// Macros for common sizes
#define KB(x) (x * 1024)
#define MB(x) (x * 1024 * 1024)
#define GB(x) (x * 1024 * 1024 * 1024)

#define _64KB	KB(64)
#define _1MB	MB(1)
#define _2MB	MB(2)
#define _4MB	MB(4)
#define _8MB	MB(8)
#define _16MB	MB(16)
#define _32MB	MB(32)
#define _64MB	MB(64)
#define _128MB	MB(128)
#define _256MB	MB(256)


const static bool FULL_SCREEN = false;
const static unsigned int WINDOW_WIDTH = 1024;
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
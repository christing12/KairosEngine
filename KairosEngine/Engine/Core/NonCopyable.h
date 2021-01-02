#pragma once


#include <Core/EngineCore.h>

KRS_BEGIN_NAMESPACE(Kairos)

class NonCopyable {
public:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable(NonCopyable&&) = delete;
protected:
	NonCopyable() = default;
	virtual ~NonCopyable() = default;

	NonCopyable& operator=(const NonCopyable&) = default;
	NonCopyable& operator=(NonCopyable&&) = default;
};

KRS_END_NAMESPACE
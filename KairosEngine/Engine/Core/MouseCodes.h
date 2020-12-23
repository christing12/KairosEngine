
#pragma once

#include <Core/EngineCore.h>

KRS_BEGIN_NAMESPACE(Kairos)


using MouseCode = unsigned int;

KRS_BEGIN_NAMESPACE(Mouse)


	enum : MouseCode
	{
		// WinUser.h
		LeftButton = 1,
		RightButton = 2,
		MiddleButton = 16
	};


KRS_END_NAMESPACE


KRS_END_NAMESPACE
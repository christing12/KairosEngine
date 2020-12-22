
#pragma once

#include <Core/EngineCore.h>

KRS_BEGIN_NAMESPACE(Kairos)


using MouseCode = unsigned int;

KRS_BEGIN_NAMESPACE(Mouse)


	enum : MouseCode
	{
		// WinUser.h
		LeftButton = 0x0001,
		RightButton = 0x0002,
		MiddleButton = 0x0010
	};


KRS_END_NAMESPACE


KRS_END_NAMESPACE
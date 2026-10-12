#pragma once
class Debug
{
public:
#ifdef _DEBUG
	static constexpr bool EnableDebug = true;
#else
	static constexpr bool EnableDebug = false;
#endif


};


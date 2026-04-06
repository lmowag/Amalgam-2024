#pragma once
#include "../../../SDK/SDK.h"

class CRadar
{
	bool GetDrawPosition(CTFPlayer* pLocal, CBaseEntity* pEntity, int& x, int& y, int& z);
	void DrawBackground();
	void DrawPoints(CTFPlayer* pLocal);

public:
	void Run();
};

ADD_FEATURE(CRadar, Radar);

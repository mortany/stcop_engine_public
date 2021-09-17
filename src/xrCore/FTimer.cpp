#include "stdafx.h"
#include "FTimer.h"

XRCORE_API BOOL g_bEnableStatGather = FALSE;

CStatTimer::CStatTimer()
{
    accum = 0;
    result = 0.f;
    count = 0;
}

void CStatTimer::FrameStart()
{
    accum = 0;
    count = 0;
}
void CStatTimer::FrameEnd()
{
    float _time = 1000.f * float(double(accum) / double(CPU::QPC_Freq()));
    if (_time > result) result = _time;
    else result = 0.99f * result + 0.01f * _time;
}

XRCORE_API pauseMngr g_pauseMngr;

ScopeStatTimer::ScopeStatTimer(CStatTimer& destTimer)
    : _timer(destTimer)
{
    _timer.Begin();
}

ScopeStatTimer::~ScopeStatTimer()
{
    _timer.End();
}
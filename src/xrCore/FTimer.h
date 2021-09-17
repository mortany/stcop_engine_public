#pragma once

class CTimerBase
{
protected:
    u64 qwStartTime;
    u64 qwPausedTime;
    u64 qwPauseAccum;
    BOOL bPause;
public:
    constexpr  CTimerBase() noexcept : qwStartTime(0), qwPausedTime(0), qwPauseAccum(0), bPause(FALSE) { }

    ICF void Start()
    {
        if (bPause)
            return;
        qwStartTime = CPU::QPC() - qwPauseAccum;
    }

    ICF u64 GetElapsed_ticks()const
    {
        if (bPause)
        {
            return qwPausedTime;
        }
        else
        {
            return CPU::QPC() - qwStartTime - qwPauseAccum;
        }
    }

    ICF u32 GetElapsed_ms()const
    {
        return u32(GetElapsed_ticks() * u64(1000) / CPU::QPC_Freq());
    }

    ICF float GetElapsed_sec()const
    {
        float _result = float(double(GetElapsed_ticks()) / double(CPU::QPC_Freq()));
        return _result;
    }

    ICF void Dump() const
    {
        Msg("* Elapsed time (sec): %f", GetElapsed_sec());
    }
};

class CTimer : public CTimerBase
{
private:
    using inherited = CTimerBase;

private:
    float m_time_factor;
    u64 m_real_ticks;
    u64 m_ticks;

private:
    ICF u64 GetElapsed_ticks(const u64& current_ticks) const
    {
        u64 delta = current_ticks - m_real_ticks;
        double delta_d = (double)delta;
        double time_factor_d = time_factor();
        double time = delta_d * time_factor_d + .5;
        u64 result = (u64)time;
        return (m_ticks + result);
    }

public:
    constexpr CTimer() noexcept : m_time_factor(1.f), m_real_ticks(0), m_ticks(0) {}

    ICF void Start() noexcept
    {
        if (bPause)
            return;

        inherited::Start();

        m_real_ticks = 0;
        m_ticks = 0;
    }

    IC const float& time_factor() const noexcept
    {
        return (m_time_factor);
    }

    ICF void time_factor(const float& time_factor) noexcept
    {
        u64 current = inherited::GetElapsed_ticks();
        m_ticks = GetElapsed_ticks(current);
        m_real_ticks = current;
        m_time_factor = time_factor;
    }

    ICF u64 GetElapsed_ticks() const
    {
        u64 result = GetElapsed_ticks(inherited::GetElapsed_ticks());

        return (result);
    }

    ICF u32 GetElapsed_ms() const
    {
        return (u32(GetElapsed_ticks() * u64(1000) / CPU::QPC_Freq()));
    }

    ICF float GetElapsed_sec() const
    {
        float result = float(double(GetElapsed_ticks()) / double(CPU::QPC_Freq()));
        return (result);
    }

    ICF void Dump() const
    {
        Msg("* Elapsed time (sec): %f", GetElapsed_sec());
    }
};

class CTimer_paused_ex : public CTimer
{
    u64 save_clock;
public:
    CTimer_paused_ex() noexcept { }
    virtual ~CTimer_paused_ex() { }
    ICF BOOL Paused()const { return bPause; }
    ICF void Pause(BOOL b)
    {
        if (bPause == b) return;

        u64 _current = CPU::QPC();
        if (b)
        {
            save_clock = _current;
            qwPausedTime = CTimerBase::GetElapsed_ticks();
        }
        else
        {
            qwPauseAccum += _current - save_clock;
        }
        bPause = b;
    }
};

class CTimer_paused;

class pauseMngr {
    xr_vector<CTimer_paused_ex*> m_timers;
    bool paused;
public:
    pauseMngr() :paused(false)
    {
        m_timers.reserve(3);
    }
    ICF bool Paused() const noexcept { return paused; }
    ICF void Pause(const bool b) noexcept
    {
        if (paused == b)return;

        xr_vector<CTimer_paused_ex*>::iterator it = m_timers.begin();
        for (; it != m_timers.end(); ++it)
            (*it)->Pause(b);

        paused = b;
    }

    ICF void Register(CTimer_paused* t)
    {
        m_timers.push_back(reinterpret_cast<CTimer_paused_ex*> (t));
    }

    ICF void UnRegister(CTimer_paused* t)
    {
        CTimer_paused_ex* timer = reinterpret_cast<CTimer_paused_ex*>(t);
        xr_vector<CTimer_paused_ex*>::iterator it = std::find(m_timers.begin(), m_timers.end(), timer);
        if (it != m_timers.end())
            m_timers.erase(it);
    }
};

extern XRCORE_API pauseMngr g_pauseMngr;

class CTimer_paused : public CTimer_paused_ex {
public:
    ICF CTimer_paused() { g_pauseMngr.Register(this); }
    ICF virtual ~CTimer_paused() { g_pauseMngr.UnRegister(this); }
};


extern XRCORE_API BOOL g_bEnableStatGather;

class XRCORE_API CStatTimer
{
public:
    CTimer T;
    u64 accum;
    float result;
    u32 count;
public:
    CStatTimer();
    void FrameStart();
    void FrameEnd();

    ICF void Begin() { if (!g_bEnableStatGather) return; count++; T.Start(); }
    ICF void End() { if (!g_bEnableStatGather) return; accum += T.GetElapsed_ticks(); }

    ICF u64 GetElapsed_ticks()const { return accum; }

    IC u32 GetElapsed_ms()const { return u32(GetElapsed_ticks() * u64(1000) / CPU::QPC_Freq()); }
    IC float GetElapsed_sec()const
    {
        float _result = float(double(GetElapsed_ticks()) / double(CPU::QPC_Freq()));
        return _result;
    }
};

struct XRCORE_API ScopeStatTimer
{
    ScopeStatTimer(CStatTimer& destTimer);
    ~ScopeStatTimer();

private:

    CStatTimer& _timer;
};
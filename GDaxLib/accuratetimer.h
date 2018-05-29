#ifndef ACCURATETIMER_H
#define ACCURATETIMER_H

#include <chrono>

class AccurateTimer
{
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point startValue;

public:
    AccurateTimer()
    {
        Reset();
    }

    void Reset()
    {
        startValue = Clock::now();
    }

    template<typename Rep, typename Period>
    std::chrono::duration<Rep, Period> Elapsed() const
    {
        auto now = Clock::now();
        return now - startValue;
    }

    double ElapsedSeconds() const
    {
        auto now = Clock::now();
        std::chrono::duration<double> elapsedSeconds = now - startValue;
        return elapsedSeconds.count();
    }
};

#endif // ACCURATETIMER_H

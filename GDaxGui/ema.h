#ifndef EMA_H
#define EMA_H

#include <QPainterPath>

#include <deque>

class Ema
{
    size_t const periods;
    double const multiplier = 2./(periods+1);
    size_t count;
    double ema;
    QPainterPath path;

public:
    Ema(size_t periods)
        : periods(periods)
        , count()
        , ema()
    {
    }

    const QPainterPath & Path() const { return path; }

    void Reset()
    {
        count = 0;
        ema = 0;
        path = {};
    }

    void Add(double x, double value)
    {
        if (count == periods)
        {
            ema = (value - ema) * multiplier + ema;
            path.lineTo({x, ema});
        }
        else if (count++ == 0)
        {
            ema += value/periods;
            path.moveTo({x, value});
        }
        else
        {
            ema += value/periods;
            path.lineTo({x, value});
        }
    }
};

#endif // EMA_H

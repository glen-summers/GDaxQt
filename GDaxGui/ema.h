#ifndef EMA_H
#define EMA_H

class Ema
{
    QPainterPath & path;
    size_t const periods;
    double multiplier = 2./(periods+1);
    size_t count;
    double ema;

public:
    Ema(QPainterPath & path, size_t periods)
        : path(path)
        , periods(periods)
        , count()
        , ema()
    {
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

#ifndef EMA_H
#define EMA_H

#include <QPainterPath>

class Ema
{
    size_t const periods;
    double const multiplier = 2./(periods+1);
    unsigned int count;
    double ema, previous;
    QPainterPath path;

public:
    Ema(size_t periods)
        : periods(periods)
        , count()
        , ema()
        , previous()
    {
    }

    const QPainterPath & Path() const { return path; }

    void Reset()
    {
        count = 0;
        ema = previous = 0;
        path = {};
    }

    double Add(double x, double value)
    {
        double newValue = Calc(value);
        AddToPath(x, newValue);
        ++count;
        return newValue;
    }

    double SetCurrentValue(double x, double value)
    {
        double newValue = 0;
        if (count != 0)
        {
            Undo();
            newValue = Calc(value);
            ModifyPath(x, newValue);
            ++count;
        }
        return newValue;
    }

private:
    void Undo()
    {
        if (count == 0)
        {
            return;
        }

        if (count >= periods)
        {
            ema = previous;
        }
        else
        {
            ema -= previous/periods;
        }
        --count;
    }

    double Calc(double value)
    {
        previous = ema;
        if (count >= periods)
        {
            return ema = (value - ema) * multiplier + ema;
        }
        else
        {
            ema += value/periods;
        }
        return value;
    }

    void AddToPath(double x, double y)
    {
        if (count == 0)
        {
            path.moveTo({x, y});
        }
        else
        {
            path.lineTo({x, y});
        }
    }

    void ModifyPath(double x, double y)
    {
        path.setElementPositionAt(count, x, y);
    }
};

#endif // EMA_H

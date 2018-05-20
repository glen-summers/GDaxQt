#ifndef SMA_H
#define SMA_H

#include <QPainterPath>

#include <deque>

class Sma
{
    size_t const periods;
    unsigned int  count;
    double total;
    std::deque<double> buffer;
    double previous;
    QPainterPath path;

public:
    Sma(size_t periods)
        : periods(periods)
        , count()
        , total()
        , previous()
    {
    }

    const QPainterPath & Path() const { return path; }

    void Reset()
    {
        count = 0;
        total = 0;
        previous = 0;
        buffer.clear();
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
            buffer.push_back(previous);
            total += previous;
        }
        total -= buffer.front();
        buffer.pop_front();
        --count;
    }

    double Calc(double value)
    {
        buffer.push_front(value);
        total += value;

        if (count >= periods)
        {
            previous = buffer.back();
            total -= previous;
            buffer.pop_back();
            return total/periods;
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

#endif // SMA_H

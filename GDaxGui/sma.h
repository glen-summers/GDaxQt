#ifndef SMA_H
#define SMA_H

#include <QPainterPath>

#include <deque>

class Sma
{
    size_t const periods;
    size_t count;
    double total;
    std::deque<double> buffer;
    QPainterPath path;

public:
    Sma(size_t periods)
        : periods(periods)
        , count()
        , total()
    {
    }

    const QPainterPath & Path() const { return path; }

    void Reset()
    {
        count = 0;
        total = 0;
        buffer.clear();
        path = {};
    }

    void Add(double x, double value)
    {
        buffer.push_front(value);
        total += value;

        if (count == periods)
        {
            total -= buffer.back();
            buffer.pop_back();
            path.lineTo({x, total/periods});
        }
        else if (count++ == 0)
        {
            path.moveTo({x, value});
        }
        else
        {
            path.lineTo({x, value});
        }
    }
};

#endif // SMA_H

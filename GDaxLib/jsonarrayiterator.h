#ifndef JSONARRAYITERATOR_H
#define JSONARRAYITERATOR_H

#include <QJsonArray>
#include <QJsonObject>

#include <iterator>

template <typename T>
class JsonArrayIterator
{
    const QJsonArray & array;
    int index;
    T value;

public:
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = void;
    using pointer = T*;
    using reference = T&;

    JsonArrayIterator(const QJsonArray & array, int index)
        : array(array), index(index), value()
    {
        if (index!=array.size())
        {
            value = T::FromJson(array[index].toObject());
        }
    }

    bool operator==(const JsonArrayIterator& it) const { return &array == &it.array && index==it.index; }
    bool operator!=(const JsonArrayIterator& it) const { return !(*this == it); }

    JsonArrayIterator& operator++()
    {
        ++index;
        if (index!=array.size())
        {
            value = T::FromJson(array[index].toObject());
        }
        else
        {
            value = {};
        }
        return *this;
    }

    const T & operator*() const { return value; }
    const T * operator->() const { return &value; }
};

#endif // JSONARRAYITERATOR_H

#ifndef JSONARRAYRESULT_H
#define JSONARRAYRESULT_H

#include "result.h"
#include "jsonarrayiterator.h"

#include <QJsonDocument>
#include <QJsonArray>

template <typename T, typename Converter = T>
class IterableResult : public Result
{
    QJsonArray const array;

public:
    typedef T Type;
    typedef Converter ConverterType;

    typedef JsonArrayIterator<T, Converter> iterator;

    IterableResult(QNetworkReply * reply)
        : Result(reply)
        , array(HasError() ? QJsonArray() : QJsonDocument::fromJson(reply->readAll()).array())
    {}

    iterator begin() const
    {
        Verify();
        return {array, 0};
    }

    const iterator end() const
    {
        return {array, array.size()};
    }

    size_t count() const { return array.size(); }
};

#endif // JSONARRAYRESULT_H

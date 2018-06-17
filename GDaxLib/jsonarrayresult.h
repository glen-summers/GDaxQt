#ifndef JSONARRAYRESULT_H
#define JSONARRAYRESULT_H

#include "result.h"
#include "jsonarrayiterator.h"

#include <QJsonDocument>
#include <QJsonArray>

// expose to client via interface?
// template <typename T> struct IIterableResult<T> {};

template <typename T, typename Converter = T>
class JsonArrayResult : public Result
{
    QJsonArray const array;

public:
    typedef JsonArrayIterator<T, Converter> iterator;

    JsonArrayResult(QNetworkReply * reply)
        : Result(reply)
        , array(HasError() ? QJsonArray() : QJsonDocument::fromJson(reply->readAll()).array())
    {}

    iterator begin() const
    {
        return {array, 0};
    }

    const iterator end() const
    {
        return {array, array.size()};
    }
};

#endif // JSONARRAYRESULT_H

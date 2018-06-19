#ifndef GENERICRESULT_H
#define GENERICRESULT_H

#include "result.h"

#include "utils.h"

#include <QJsonDocument>

template <typename T, typename Converter = T>
class GenericResult : public Result
{
    T const value;

public:
    typedef T Type;
    typedef Converter ConverterType;

    GenericResult(QNetworkReply * reply)
        : Result(reply)
        , value(HasError() ? T() : Converter::FromJson(QJsonDocument::fromJson(reply->readAll()).object()))
    {}

    const T & operator()() const
    {
        Verify();
        return value;
    }
};

#endif // GENERICRESULT_H

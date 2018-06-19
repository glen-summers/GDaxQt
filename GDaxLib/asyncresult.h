#ifndef ASYNCRESULT_H
#define ASYNCRESULT_H

#include "genericresult.h"

// assumptions: only works with same thread qobjects and state not changin during construction\assignment
template <typename Result>
class AsyncResultImpl : public QObject
{
    std::function<void(const Result&)> func;

public:
    AsyncResultImpl(QNetworkReply * reply) : QObject(reply)
    {
        QObject::connect(reply, &QNetworkReply::finished, this, &AsyncResultImpl::Finished);
    }

    template <typename Func> AsyncResultImpl & Then(Func f) { func = f; return *this; }

private:
    void Finished()
    {
        QNetworkReply * reply = static_cast<QNetworkReply *>(parent());
        if (func)
        {
            func(Result{reply});
        }
        reply->deleteLater();
    }
};

template <typename Result>
class Async
{
    typedef AsyncResultImpl<Result> ResultImpl;
    ResultImpl * result;

public:
    Async(QNetworkReply * r) : result(Utils::QMake<ResultImpl>("AsyncResult", r))
    {}

    template <typename Func> Async& Then(Func f) { result->Then(f); return *this; }
};


#endif // ASYNCRESULT_H

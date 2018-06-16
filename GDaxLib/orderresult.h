#ifndef ORDERRESULT_H
#define ORDERRESULT_H

#include "result.h"
#include "order.h"

#include <QString>
#include <QNetworkReply>
#include <QJsonArray>

class OrderResult : public Result
{
    QJsonArray const array;

public:
    OrderResult(QNetworkReply * reply);

    class Iterator : public std::iterator<std::input_iterator_tag, Order>
    {
        const QJsonArray & array;
        int index;
        Order order;

    public:
        Iterator(const QJsonArray & array, int index);

        bool operator==(const Iterator& it) const { return &array == &it.array && index==it.index; }
        bool operator!=(const Iterator& it) const { return !(*this == it); }
        Iterator& operator++();
        const Order & operator*() const { return order; }
        const Order * operator->() const { return &order; }
    };

    Iterator begin() const;
    const Iterator end() const;
};

#endif // ORDERRESULT_H

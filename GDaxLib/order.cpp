#include "order.h"

#include <QJsonObject>

Order Order::FromJson(const QJsonObject & object)
{
    return { object["id"].toString() };
    // ...
}

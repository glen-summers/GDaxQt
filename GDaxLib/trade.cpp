#include "trade.h"

#include <QDateTime>
#include <QJsonObject>

Trade Trade::FromJson(const QJsonValue & value)
{
    auto object = value.toObject();
    // truncates us? try chrono
    QDateTime time = QDateTime::fromString(object["time"].toString(), Qt::ISODateWithMs);
    auto tradeId = static_cast<TradeId>(object["trade_id"].toDouble());
    Decimal price(object["price"].toString().toStdString());
    Decimal size(object["size"].toString().toStdString());

    // The trade side indicates the maker order side. The maker order is the order that was open on the order book.
    // buy side indicates a down-tick because the maker was a buy order and their order was removed.
    // Conversely, sell side indicates an up-tick
    MakerSide side = MakerSideFromString(object["side"].toString().toStdString());
    return { time, tradeId, price, size, side };
}

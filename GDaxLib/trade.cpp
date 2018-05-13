#include "trade.h"

#include <QDateTime>

Trade Trade::fromJson(const QJsonObject & object)
{
    // truncates us? try chrono
    QDateTime time = QDateTime::fromString(object["time"].toString(), Qt::ISODateWithMs);
    auto tradeId = static_cast<TradeId>(object["trade_id"].toDouble());
    Decimal price(object["price"].toString().toStdString());
    Decimal size(object["size"].toString().toStdString());

    // The trade side indicates the maker order side. The maker order is the order that was open on the order book.
    // buy side indicates a down-tick because the maker was a buy order and their order was removed.
    // Conversely, sell side indicates an up-tick
    // rework as MakerSide -> Up\Down, and TakerSide - > Down\Up
    QString sideValue = object["side"].toString();
    MakerSide side = sideValue.isEmpty()
            ? MakerSide::None
            : sideValue == "buy"
                ? MakerSide::Buy
                : sideValue == "sell"
                    ? MakerSide::Sell
                    : throw std::runtime_error("Unexpected side");

    return { time, tradeId, price, size, side };
}

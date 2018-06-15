#include "tick.h"

#include <QJsonObject>

Tick Tick::FromJson(const QJsonObject & object)
{
    auto tradeId = static_cast<TradeId>(object["last_trade_id"].toDouble());
    auto sequence = static_cast<SequenceNumber>(object["sequence"].toDouble());

    // truncates us? try chrono
    QDateTime time = QDateTime::fromString(object["time"].toString(), Qt::ISODateWithMs);
    Decimal price(object["price"].toString().toStdString());
    TakerSide side = TakerSideFromString(object["side"].toString().toStdString());
    Decimal lastSize(object["last_size"].toString().toStdString());
    Decimal bestBid(object["best_bid"].toString().toStdString());
    Decimal bestAsk(object["best_ask"].toString().toStdString());

    return { tradeId, sequence, time, price, side, lastSize, bestBid, bestAsk };
}

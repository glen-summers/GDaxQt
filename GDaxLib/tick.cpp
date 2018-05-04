#include "tick.h"

#include <QJsonObject>

Tick Tick::fromJson(const QJsonObject & object)
{
    auto tradeId = static_cast<Tick::TradeId>(object["last_trade_id"].toDouble());
    auto sequence = static_cast<Tick::SequenceNumber>(object["sequence"].toDouble());

    // truncates us? try chrono
    QDateTime time = QDateTime::fromString(object["time"].toString(), Qt::ISODateWithMs);

    Decimal price(object["price"].toString().toUtf8().constData());
    QString sideValue = object["side"].toString();
    Tick::Side side = sideValue.isEmpty()
            ? None
            : sideValue == "buy"
                ? Buy
                : sideValue == "sell"
                    ? Sell
                    : throw std::runtime_error("Unexpected side");

    Decimal lastSize(object["last_size"].toString().toUtf8().constData());
    Decimal bestBid(object["best_bid"].toString().toUtf8().constData());
    Decimal bestAsk(object["best_ask"].toString().toUtf8().constData());

    //qInfo(QString("Ticker: %1 %2 %3 %4 %5 %6").arg(seq).arg(tradeId).arg(time, side, price, size).toUtf8().constData());

    return { tradeId, sequence, time, price, side, lastSize, bestBid, bestAsk };
}

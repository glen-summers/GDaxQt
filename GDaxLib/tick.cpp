#include "tick.h"

#include <QJsonObject>

Tick Tick::FromJson(const QJsonObject & object)
{
    auto tradeId = static_cast<TradeId>(object["last_trade_id"].toDouble());
    auto sequence = static_cast<SequenceNumber>(object["sequence"].toDouble());

    // truncates us? try chrono
    QDateTime time = QDateTime::fromString(object["time"].toString(), Qt::ISODateWithMs);

    Decimal price(object["price"].toString().toStdString());
    QString sideValue = object["side"].toString();
    TakerSide side = sideValue.isEmpty()
            ? TakerSide::None
            : sideValue == "buy"
                ? TakerSide::Buy
                : sideValue == "sell"
                    ? TakerSide::Sell
                    : throw std::runtime_error("Unexpected side");

    Decimal lastSize(object["last_size"].toString().toStdString());
    Decimal bestBid(object["best_bid"].toString().toStdString());
    Decimal bestAsk(object["best_ask"].toString().toStdString());

    //qInfo(QString("Ticker: %1 %2 %3 %4 %5 %6").arg(seq).arg(tradeId).arg(time, side, price, size).toStdString());

    return { tradeId, sequence, time, price, side, lastSize, bestBid, bestAsk };
}

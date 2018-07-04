#ifndef CONSOLETEST_H
#define CONSOLETEST_H

#include "gdl.h"

class ConsoleTest : GDL::IStreamCallbacks
{
    GDL::RequestPtr request;
    GDL::StreamPtr stream;

public:
    ConsoleTest(GDL::Auth * auth);

    void PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side) const;
    void Orders() const;
    void CancelOrders() const;

private:
    void OnSnapshot(const QString & product, const IterableResult<GDL::OrderBookItem> & bids, const IterableResult<GDL::OrderBookItem> & asks) override;
    void OnUpdate(const QString & product, const IterableResult<GDL::OrderBookChange> & changes) override;
    void OnHeartbeat(const QDateTime & serverTime) override;
    void OnTick(const Tick & tick) override;
    void OnStateChanged(GDL::ConnectedState state) override;
};

#endif // CONSOLETEST_H

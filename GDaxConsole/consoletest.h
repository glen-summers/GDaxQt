#ifndef CONSOLETEST_H
#define CONSOLETEST_H

#include "gdl.h"

class ConsoleTest : public GDL::Callback
{
    GDL::InterfacePtr gdl;

public:
    ConsoleTest();

    void PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side) const;
    void Orders() const;
    void CancelOrders() const;
    void Shutdown();

private:
    void OnSnapshot() override;
    void OnHeartbeat(const QDateTime & serverTime) override;
    void OnTick(const Tick & tick) override;
    void OnStateChanged(GDL::ConnectedState state) override;
};

#endif // CONSOLETEST_H

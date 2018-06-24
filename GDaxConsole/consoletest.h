#ifndef CONSOLETEST_H
#define CONSOLETEST_H

#include "gdl.h"

class ConsoleTest
{
    GDL::RequestPtr request;

public:
    ConsoleTest();

    void PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side) const;
    void Orders() const;
    void CancelOrders() const;
    void Shutdown();
};

#endif // CONSOLETEST_H

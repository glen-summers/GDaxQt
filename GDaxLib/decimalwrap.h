#ifndef DECIMALWRAP_H
#define DECIMALWRAP_H

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(push,3)
#endif

#define DEC_NAMESPACE DecNs
#include "decimal.h"
#undef DEC_NAMESPACE

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// 64 bit impl == 18 digits
typedef DecNs::decimal<8> Decimal;

#endif // DECIMALWRAP_H

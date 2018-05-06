#ifndef DEFS_H
#define DEFS_H

#define DEC_NAMESPACE DecNs
#include <decimal.h>

// 64 bit impl == 18 digits
typedef DecNs::decimal<8> Decimal;

#endif // DEFS_H

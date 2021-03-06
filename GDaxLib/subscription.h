#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <vector>
#include <string>

namespace Channel
{
    extern const char Level2[];
    extern const char Full[];
    extern const char Heartbeat[];
    extern const char Ticker[];
    extern const char Matches[];
    extern const char User[];
};

struct Subscription
{
    std::vector<std::string> ProductIds;
    std::vector<std::string> Channels;
    // or\and Channel|product pairs
};

#endif // SUBSCRIPTION_H

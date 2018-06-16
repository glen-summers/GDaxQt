#ifndef SERVERTIMERESULT_H
#define SERVERTIMERESULT_H

#include "result.h"
#include <QDateTime>

class ServerTimeResult : public Result
{
    QDateTime const value;

public:
    ServerTimeResult(QNetworkReply * reply);
    const QDateTime & Value() const { return value; }
};

#endif // SERVERTIMERESULT_H

#ifndef RESULT_H
#define RESULT_H

#include <QNetworkReply>
#include <QString>

class Result
{
    QNetworkReply::NetworkError const error;
    QString const errorString;

public:
    Result(QNetworkReply * reply)
        : error(reply->error())
        , errorString(reply->errorString())
    {}

    QNetworkReply::NetworkError Error() const { return error; }
    const QString & ErrorString() const { return errorString; }
    bool HasError() const { return error != QNetworkReply::NetworkError::NoError; }
};


#endif // RESULT_H

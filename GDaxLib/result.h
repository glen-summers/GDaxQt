#ifndef RESULT_H
#define RESULT_H

#include <QNetworkReply>
#include <QString>

// make a generic JsonArray Result?
class Result
{
    QNetworkReply::NetworkError const error;
    QString const errorString;

public:
    Result(QNetworkReply * reply);

    QNetworkReply::NetworkError Error() const { return error; }
    QString ErrorString() const;
    bool HasError() const { return error != QNetworkReply::NetworkError::NoError; }
};


#endif // RESULT_H

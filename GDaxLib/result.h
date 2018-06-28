#ifndef RESULT_H
#define RESULT_H

#include <QNetworkReply>
#include <QString>

class Result
{
    QNetworkReply::NetworkError const error;
    QString const errorString;

public:
    Result(QNetworkReply * reply = nullptr);

    QNetworkReply::NetworkError Error() const { return error; }
    std::string ErrorString() const;
    bool HasError() const { return error != QNetworkReply::NetworkError::NoError; }

    void Verify() const;
};


#endif // RESULT_H

#ifndef RESTPROVIDER_H
#define RESTPROVIDER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>

class RestProvider : public QObject
{
    Q_OBJECT

    QNetworkAccessManager manager;

public:
    RestProvider();

private slots:
    void error(QNetworkReply::NetworkError error);
    void sslErrors(QList<QSslError> errors);
    void downloadFinished(QNetworkReply * reply);
};

#endif // RESTPROVIDER_H

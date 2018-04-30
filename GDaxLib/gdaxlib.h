#ifndef GDAXLIB_H
#define GDAXLIB_H

// discover/relative path/ or use unofficial QDecimal
#define DEC_NAMESPACE DecNs
#include <C:\Users\Glen\source\github\decimal_for_cpp\include\decimal.h>

#include <QObject>
#include <QWebSocket>
QT_FORWARD_DECLARE_CLASS(QJsonObject)

#include <map>

class GDaxLib : public QObject
{
    typedef DecNs::decimal<10> Decimal;

    Q_OBJECT

    QWebSocket webSocket;

    std::map<Decimal, Decimal> bids, asks;
    Decimal priceMin;
    Decimal priceMax;
    Decimal amountMax;

public:
    explicit GDaxLib(QObject * parent = nullptr);

private Q_SLOTS:
    void onConnected();

    void onTextMessageReceived(QString message);
    void onTextFrameReceived(QString message, bool isLastFrame);
    void onBinaryMessageReceived(const QByteArray &message);
    void onBinaryFrameReceived(const QByteArray &message, bool isLastFrame);

    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);

    void ProcessSnapshot(const QJsonObject & object);
    void ProcessUpdate(const QJsonObject & object);
};

#endif // GDAXLIB_H

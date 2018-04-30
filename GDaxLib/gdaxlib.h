#ifndef GDAXLIB_H
#define GDAXLIB_H

#include <QObject>
//QT_FORWARD_DECLARE_CLASS(QWebSocket)
#include <QWebSocket> // fwd

class GDaxLib : public QObject
{
    static constexpr const char * url = "wss://ws-feed.gdax.com";

    static constexpr const char * subscribeMessage = R"(
    {
    "type": "subscribe",
    "product_ids": [
        "BTC-EUR"
    ],
    "channels": [
        "level2"
    ]
})";

    Q_OBJECT

    QWebSocket webSocket;

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
};

#endif // GDAXLIB_H

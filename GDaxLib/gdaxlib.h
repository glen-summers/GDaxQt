#ifndef GDAXLIB_H
#define GDAXLIB_H

#include <QObject>
//QT_FORWARD_DECLARE_CLASS(QWebSocket)
#include <QWebSocket> // fwd

class GDaxLib : public QObject
{
    Q_OBJECT

    QWebSocket webSocket;

public:
    explicit GDaxLib(const QString & url, QObject * parent = nullptr);

private Q_SLOTS:
    void onConnected();
    void onTextMessageReceived(QString message);
    void onSslErrors(const QList<QSslError> &errors);
};

#endif // GDAXLIB_H

#ifndef WEBSOCKETSTREAM_H
#define WEBSOCKETSTREAM_H

#include "defs.h"
#include "gdl.h" //fwd?
#include "subscription.h"

#include "flogging.h"

#include <QObject>
#include <unordered_map>

struct Tick;
class Authenticator;

class QWebSocket;
class QJsonObject;
class QTimer;

class WebSocketStream : public QObject, public GDL::IStream
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<WebSocketStream>();

    Q_OBJECT

    typedef std::unordered_map<std::string, void(WebSocketStream::*)(const QJsonObject & object)> FunctionMap;
    static FunctionMap functionMap;

    GDL::IStreamCallbacks & callback;
    QString const url;
    Subscription const subscription;
    QWebSocket * const webSocket;
    QThread * const workerThread;
    QTimer * const pingTimer;
    std::unique_ptr<Authenticator> authenticator;

    TradeId lastTradeId;

public:
    explicit WebSocketStream(const char * url, const Subscription & subscription, GDL::IStreamCallbacks & callback);

    void SetAuthentication(const char key[], const char secret[], const char passphrase[]) override;
    void ClearAuthentication() override;

    //void Subscribe(const Subscription & subscription) override;
    //void Unsubscribe(const Subscription & subscription) override;

    void Shutdown() override;

private slots:
    void Connected();
    void TextMessageReceived(QString message);
    void Ping();
    void Pong();

private:
    void Open();
    void Clear();
    void ProcessSubscriptions(const QJsonObject & object);
    void ProcessError(const QJsonObject & object);
    void ProcessSnapshot(const QJsonObject & object);
    void ProcessUpdate(const QJsonObject & object);
    void ProcessHeartbeat(const QJsonObject & object);
    void ProcessTicker(const QJsonObject & object);

    void ProcessReceived(const QJsonObject & object);
    void ProcessOpen(const QJsonObject & object);
    void ProcessDone(const QJsonObject & object);
};

#endif // WEBSOCKETSTREAM_H

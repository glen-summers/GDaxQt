#include "gdaxlib.h"


GDaxLib::GDaxLib(const QString & url, QObject * parent) // parent?
    : QObject(parent)
{
    connect(&webSocket, &QWebSocket::connected, this, &GDaxLib::onConnected);

    typedef void (QWebSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    connect(&webSocket, static_cast<sslErrorsSignal>(&QWebSocket::sslErrors), this, &GDaxLib::onSslErrors);
    webSocket.open(QUrl(url));
}

void GDaxLib::onConnected()
{
    // need qInstallMessageHandler(SyslogMessageHandler);? and handle ourselves?
    // https://stackoverflow.com/questions/28540571/how-to-enable-and-disable-qdebug-messages
    // https://gist.github.com/polovik/10714049
    qInfo("onConnected");

    connect(&webSocket, &QWebSocket::textMessageReceived, this, &GDaxLib::onTextMessageReceived);
    webSocket.sendTextMessage(QStringLiteral("hello"));
}

void GDaxLib::onTextMessageReceived(QString message)
{
    qInfo((std::string("msg:") + message.toUtf8().constData()).c_str());
}

void GDaxLib::onSslErrors(const QList<QSslError> &errors)
{
    for(auto & e : errors)
    {
        qWarning(e.errorString().toUtf8().constData());
    }
}


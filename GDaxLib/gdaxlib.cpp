#include "gdaxlib.h"


GDaxLib::GDaxLib(QObject * parent) // parent?
    : QObject(parent)
{
    connect(&webSocket, &QWebSocket::connected, this, &GDaxLib::onConnected);

    typedef void (QWebSocket:: *sslErrorsSignal)(const QList<QSslError> &);

    connect(&webSocket, &QWebSocket::textMessageReceived, this, &GDaxLib::onTextMessageReceived);
    connect(&webSocket, &QWebSocket::textFrameReceived, this, &GDaxLib::onTextFrameReceived);
    connect(&webSocket, &QWebSocket::binaryFrameReceived, this, &GDaxLib::onBinaryFrameReceived);
    connect(&webSocket, &QWebSocket::binaryMessageReceived, this, &GDaxLib::onBinaryMessageReceived);
    connect(&webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &GDaxLib::onError);
    connect(&webSocket, static_cast<sslErrorsSignal>(&QWebSocket::sslErrors), this, &GDaxLib::onSslErrors);

    webSocket.open(QUrl(url));
}

void GDaxLib::onConnected()
{
    // need qInstallMessageHandler(SyslogMessageHandler);? and handle ourselves?
    // https://stackoverflow.com/questions/28540571/how-to-enable-and-disable-qdebug-messages
    // https://gist.github.com/polovik/10714049
    qInfo("onConnected");

    webSocket.sendTextMessage(subscribeMessage);
}

void GDaxLib::onTextMessageReceived(QString message)
{
    qInfo((std::string("textMsg:") + message.toUtf8().constData()).c_str());
}

void GDaxLib::onTextFrameReceived(QString message, bool isLastFrame)
{
    qInfo((std::string("textFrame:") + message.toUtf8().constData()).c_str());
}

void GDaxLib::onBinaryMessageReceived(const QByteArray &message)
{
    qInfo("binMsg");
}

void GDaxLib::onBinaryFrameReceived(const QByteArray &message, bool isLastFrame)
{
    qInfo("binFrame");
}

void GDaxLib::onError(QAbstractSocket::SocketError error)
{
    qWarning(std::to_string((int)error).c_str());
}

void GDaxLib::onSslErrors(const QList<QSslError> &errors)
{
    for(auto & e : errors)
    {
        qWarning(e.errorString().toUtf8().constData());
    }
}


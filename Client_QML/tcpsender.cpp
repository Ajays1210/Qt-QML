#include "tcpsender.h"

#include <QHostAddress>
#include <QDebug>

TcpSender::TcpSender(QObject *parent)
    : QObject(parent),
    m_socket(nullptr)   // no socket yet; created later in the right thread
{
}

// Runs inside the sender thread. Creating the socket HERE (not in the
// constructor) makes it belong to this thread.
void TcpSender::connectToServer()
{
    m_socket = new QTcpSocket(this);

    // 127.0.0.1 = this same PC (the server runs on the same machine).
    m_socket->connectToHost(QHostAddress::LocalHost, 4001);

    // Wait up to 3 seconds. This blocks, but only the sender thread,
    // never the GUI.
    if (m_socket->waitForConnected(3000))
        qInfo() << "Client: connected to TCP 4001";
    else
        qWarning() << "Client: TCP connection failed:" << m_socket->errorString();
}

// Sends one command (the raw struct bytes) to the server.
void TcpSender::sendCommand(QByteArray data)
{
    // Safety check: connectToServer has not created the socket yet.
    if (m_socket == nullptr)
        return;

    // If the connection was lost (or the server was not running at startup),
    // try to connect again before sending.
    if (m_socket->state() != QAbstractSocket::ConnectedState)
    {
        m_socket->connectToHost(QHostAddress::LocalHost, 4001);

        if (!m_socket->waitForConnected(1000)) {
            qWarning() << "Client: reconnect failed";
            return;   // give up for this command
        }
    }

    m_socket->write(data);
    m_socket->flush();   // push the bytes out now instead of waiting
}
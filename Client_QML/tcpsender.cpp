#include "tcpsender.h"

#include <QHostAddress>
#include <QDebug>

TcpSender::TcpSender(QObject *parent)
    : QObject(parent),
    m_socket(nullptr)
{
}

void TcpSender::connectToServer()
{
    m_socket = new QTcpSocket(this);
    m_socket->connectToHost(QHostAddress::LocalHost, 4001);

    if (m_socket->waitForConnected(3000))
        qInfo() << "Client: connected to TCP 4001";
    else
        qWarning() << "Client: TCP connection failed:" << m_socket->errorString();
}

void TcpSender::sendCommand(QByteArray data)
{
    if (m_socket == nullptr)
        return;

    if (m_socket->state() != QAbstractSocket::ConnectedState)
    {
        m_socket->connectToHost(QHostAddress::LocalHost, 4001);

        if (!m_socket->waitForConnected(1000)) {
            qWarning() << "Client: reconnect failed";
            return;
        }
    }

    m_socket->write(data);
    m_socket->flush();
}
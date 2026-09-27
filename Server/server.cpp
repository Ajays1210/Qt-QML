#include "server.h"
#include "tcplistener.h"
#include "udpbroadcaster.h"

Server::Server(QObject *parent)
    : QObject(parent)
{
    m_tcpListener = new TcpListener;
    m_udpBroadcaster = new UdpBroadcaster;

    m_tcpListener->moveToThread(&m_tcpThread);
    m_udpBroadcaster->moveToThread(&m_udpThread);

    connect(&m_tcpThread, &QThread::started,
            m_tcpListener, &TcpListener::start);

    connect(&m_udpThread, &QThread::started,
            m_udpBroadcaster, &UdpBroadcaster::start);

    connect(m_tcpListener, &TcpListener::addResponse,
            m_udpBroadcaster, &UdpBroadcaster::broadcastAddResponse,
            Qt::QueuedConnection);

    connect(m_tcpListener, &TcpListener::updateResponse,
            m_udpBroadcaster, &UdpBroadcaster::broadcastUpdateResponse,
            Qt::QueuedConnection);

    connect(m_tcpListener, &TcpListener::deleteResponse,
            m_udpBroadcaster, &UdpBroadcaster::broadcastDeleteResponse,
            Qt::QueuedConnection);

    connect(m_tcpListener, &TcpListener::logMessage,
            this, [](const QString &message) {
                qInfo().noquote() << "[TCP]" << message;
            });

    connect(m_udpBroadcaster, &UdpBroadcaster::logMessage,
            this, [](const QString &message) {
                qInfo().noquote() << "[UDP]" << message;
            });

    connect(&m_tcpThread, &QThread::finished,
            m_tcpListener, &QObject::deleteLater);
    connect(&m_udpThread, &QThread::finished,
            m_udpBroadcaster, &QObject::deleteLater);
}

Server::~Server()
{
    stop();
}

void Server::start()
{
    m_udpThread.start();
    m_tcpThread.start();
}

void Server::stop()
{
    if (m_tcpThread.isRunning()) {
        QMetaObject::invokeMethod(m_tcpListener, "stop", Qt::BlockingQueuedConnection);
        m_tcpThread.quit();
        m_tcpThread.wait();
    }

    if (m_udpThread.isRunning()) {
        QMetaObject::invokeMethod(m_udpBroadcaster, "stop", Qt::BlockingQueuedConnection);
        m_udpThread.quit();
        m_udpThread.wait();
    }
}
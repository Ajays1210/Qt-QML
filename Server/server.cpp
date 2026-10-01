#include "server.h"
#include "tcplistener.h"
#include "udpbroadcaster.h"

// Server owns two worker objects, each living in its own thread:
//   TcpListener    -> receives commands on TCP 4001   (reception thread)
//   UdpBroadcaster -> sends responses on UDP 4002     (sending thread)
// This satisfies "reception and sending in two separate threads".
Server::Server(QObject *parent)
    : QObject(parent)
{
    // Created WITHOUT a parent, because an object with a parent
    // cannot be moved to another thread.
    m_tcpListener = new TcpListener;
    m_udpBroadcaster = new UdpBroadcaster;

    // After this, the objects' slots run in those threads.
    m_tcpListener->moveToThread(&m_tcpThread);
    m_udpBroadcaster->moveToThread(&m_udpThread);

    // When a thread starts, call the worker's start() inside that thread.
    // The sockets get created there, so they belong to the right thread.
    connect(&m_tcpThread, &QThread::started,
            m_tcpListener, &TcpListener::start);
    connect(&m_udpThread, &QThread::started,
            m_udpBroadcaster, &UdpBroadcaster::start);

    // KEY LINK between the two threads:
    // TcpListener emits response() -> UdpBroadcaster::broadcast() runs in the UDP thread.
    // QueuedConnection = the data is copied and delivered safely across threads.
    connect(m_tcpListener, &TcpListener::response,
            m_udpBroadcaster, &UdpBroadcaster::broadcast,
            Qt::QueuedConnection);

    // Log messages come from the worker threads, but printing is done
    // in the main thread (the lambda runs in 'this' thread).
    connect(m_tcpListener, &TcpListener::logMessage,
            this, [](const QString &m) { qInfo().noquote() << "[TCP]" << m; });
    connect(m_udpBroadcaster, &UdpBroadcaster::logMessage,
            this, [](const QString &m) { qInfo().noquote() << "[UDP]" << m; });

    // Delete the workers safely when their thread finishes.
    connect(&m_tcpThread, &QThread::finished,
            m_tcpListener, &QObject::deleteLater);
    connect(&m_udpThread, &QThread::finished,
            m_udpBroadcaster, &QObject::deleteLater);
}

Server::~Server()
{
    stop();   // safe to call twice: it checks isRunning()
}

void Server::start()
{
    m_udpThread.start();   // UDP first so it is ready before any TCP command arrives
    m_tcpThread.start();
}

void Server::stop()
{
    if (m_tcpThread.isRunning()) {
        // Ask the worker to close its sockets, inside its own thread, and wait for it.
        QMetaObject::invokeMethod(m_tcpListener, "stop", Qt::BlockingQueuedConnection);
        m_tcpThread.quit();   // stop the thread's event loop
        m_tcpThread.wait();   // block until it has fully finished
    }
    if (m_udpThread.isRunning()) {
        QMetaObject::invokeMethod(m_udpBroadcaster, "stop", Qt::BlockingQueuedConnection);
        m_udpThread.quit();
        m_udpThread.wait();
    }
}
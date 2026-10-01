#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>

#include "protocol.h"

// Lives in the NETWORK thread (created and moved there by MainWindow).
// It does ALL the socket work so the GUI never blocks:
//   - sends commands to the server on TCP 4001
//   - receives the server's responses on UDP 4002
class Network : public QObject
{
    Q_OBJECT

public:
    explicit Network(QObject *parent = nullptr);

public slots:
    // Runs when the network thread starts (QThread::started, connected in the
    // MainWindow constructor). Creates the sockets here so they belong to this
    // thread, binds UDP 4002 and connects TCP to 4001.
    // There is no stop(): the sockets are children of this object, so they are
    // closed automatically when this object is deleted at the end of the thread.
    void start();

    // Called from MainWindow (through QMetaObject::invokeMethod) when the user
    // presses Apply or confirms Delete. They run in the network thread.
    void sendAdd(unsigned int uniqueId, float lat, float longi, const QString &comment);
    void sendUpdate(unsigned int uniqueId, float lat, float longi, const QString &comment);
    void sendDelete(unsigned int uniqueId);

signals:
    // Emitted from onUdpReadyRead() when a valid response arrives.
    // Connected in the MainWindow constructor to MainWindow::onXxxResponse.
    // They carry structs between threads. Qt 6 copies them by itself
    // (Qt 5 would need qRegisterMetaType in main.cpp).
    void addResponse(Protocol::AddDataResp response);
    void updateResponse(Protocol::UpdateDataResp response);
    void deleteResponse(Protocol::DeleteDataResp response);

    // Text for the status label. Connected to MainWindow::onStatusMessage.
    void statusMessage(const QString &message);

private slots:
    void onTcpConnected();                                   // QTcpSocket::connected
    void onTcpError(QAbstractSocket::SocketError error);     // QTcpSocket::errorOccurred
    void onUdpReadyRead();                                   // QUdpSocket::readyRead

private:
    bool ensureTcpConnection();   // makes sure TCP is connected before sending

    // Builds an Add or Update command (same struct, only cmdId differs) and sends it.
    void sendRecord(unsigned char cmdId, unsigned int uniqueId,
                    float lat, float longi, const QString &comment);

    // Connects if needed, then writes raw bytes to the server.
    void sendBytes(const char *data, int size);

    QTcpSocket *m_tcp = nullptr;
    QUdpSocket *m_udp = nullptr;
};

#endif
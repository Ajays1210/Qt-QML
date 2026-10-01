#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include <QObject>
#include <QUdpSocket>

// Runs in the RECEIVER thread (set up in main.cpp). Listens on UDP port 4002
// for the server's responses, works out which type each one is, and emits a
// matching signal for Controller.
class UdpReceiver : public QObject
{
    Q_OBJECT

public:
    explicit UdpReceiver(QObject *parent = nullptr);

signals:
    // Emitted from onReadyRead() when a valid response arrives.
    // Connected in main.cpp (queued) to Controller::onXxxResponse,
    // so the list is updated in the main thread.
    void addResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment);
    void updateResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment);
    void deleteResponse(unsigned char ack, unsigned int uniqueId);

public slots:
    // Called when the receiver thread starts (QThread::started, in main.cpp).
    // Creates the socket and binds port 4002.
    void startListening();

private slots:
    // Called by QUdpSocket::readyRead whenever datagrams have arrived.
    void onReadyRead();

private:
    QUdpSocket *m_socket;   // created in startListening so it belongs to this thread
};

#endif
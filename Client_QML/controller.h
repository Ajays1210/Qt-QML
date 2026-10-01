#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QByteArray>

#include "model.h"

// Lives in the MAIN thread. It is the link between the QML screen,
// the network threads and the list model:
//   QML button -> sendAdd/Update/Delete -> sendBytes signal -> TcpSender (thread)
//   UdpReceiver (thread) -> onXxxResponse slots -> Model -> QML list updates
class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller(Model *model, QObject *parent = nullptr);

    // Q_INVOKABLE = these can be called from QML (Main.qml calls them
    // when Apply or Delete is clicked). Each one builds a command struct
    // and emits sendBytes. They do NOT touch the list.
    Q_INVOKABLE void sendAdd(unsigned int uniqueId, double lat, double longi, const QString &comment);
    Q_INVOKABLE void sendUpdate(unsigned int uniqueId, double lat, double longi, const QString &comment);
    Q_INVOKABLE void sendDelete(unsigned int uniqueId);

signals:
    // Emitted by the three send functions above.
    // Connected in main.cpp to TcpSender::sendCommand (a queued connection,
    // so it runs in the sender thread).
    void sendBytes(QByteArray data);

public slots:
    // Called (via queued signals from UdpReceiver, connected in main.cpp)
    // when the server's response arrives. They run in the main thread, so
    // it is safe for them to change the model.
    void onAddResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment);
    void onUpdateResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment);
    void onDeleteResponse(unsigned char ack, unsigned int uniqueId);

private:
    Model *m_model;   // the list data (owned by main(), not by this class)
};

#endif
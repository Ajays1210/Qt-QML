#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QByteArray>

#include "model.h"

class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller(Model *model, QObject *parent = nullptr);

    Q_INVOKABLE void sendAdd(unsigned int uniqueId, double lat, double longi, const QString &comment);
    Q_INVOKABLE void sendUpdate(unsigned int uniqueId, double lat, double longi, const QString &comment);
    Q_INVOKABLE void sendDelete(unsigned int uniqueId);

signals:
    void sendBytes(QByteArray data);

public slots:
    void onAddResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment);
    void onUpdateResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment);
    void onDeleteResponse(unsigned char ack, unsigned int uniqueId);

private:
    Model *m_model;
};

#endif
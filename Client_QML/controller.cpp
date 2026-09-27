#include "controller.h"
#include "protocol.h"

#include <cstring>

static void packComment(char *destination, unsigned int destinationSize, const QString &comment)
{
    std::memset(destination, 0, destinationSize);
    QByteArray bytes = comment.toUtf8();
    unsigned int length = static_cast<unsigned int>(bytes.size());

    if (length >= destinationSize)
        length = destinationSize - 1;

    std::memcpy(destination, bytes.constData(), length);
}

Controller::Controller(Model *model, QObject *parent)
    : QObject(parent),
    m_model(model)
{
}

void Controller::sendAdd(unsigned int uniqueId, double lat, double longi, const QString &comment)
{
    Protocol::AddDataCmd command{};
    command.cmdID = Protocol::CMD_ADD;
    command.UniqueID = uniqueId;
    command.lat = static_cast<float>(lat);
    command.longi = static_cast<float>(longi);
    packComment(command.comment, sizeof(command.comment), comment);

    QByteArray data(reinterpret_cast<const char *>(&command), sizeof(command));
    emit sendBytes(data);
}

void Controller::sendUpdate(unsigned int uniqueId, double lat, double longi, const QString &comment)
{
    Protocol::UpdateDataCmd command{};
    command.cmdID = Protocol::CMD_UPDATE;
    command.UniqueID = uniqueId;
    command.lat = static_cast<float>(lat);
    command.longi = static_cast<float>(longi);
    packComment(command.comment, sizeof(command.comment), comment);

    QByteArray data(reinterpret_cast<const char *>(&command), sizeof(command));
    emit sendBytes(data);
}

void Controller::sendDelete(unsigned int uniqueId)
{
    Protocol::DeleteDataCmd command{};
    command.cmdID = Protocol::CMD_DELETE;
    command.UniqueID = uniqueId;

    QByteArray data(reinterpret_cast<const char *>(&command), sizeof(command));
    emit sendBytes(data);
}

void Controller::onAddResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment)
{
    if (ack == Protocol::ACK_SUCCESS)
        m_model->addOrReplace(uniqueId, lat, longi, comment);
}

void Controller::onUpdateResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment)
{
    if (ack == Protocol::ACK_SUCCESS)
        m_model->addOrReplace(uniqueId, lat, longi, comment);
}

void Controller::onDeleteResponse(unsigned char ack, unsigned int uniqueId)
{
    if (ack == Protocol::ACK_SUCCESS)
        m_model->removeById(uniqueId);
}
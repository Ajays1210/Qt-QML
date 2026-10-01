#include "controller.h"
#include "protocol.h"

#include <cstring>

// Copies a QString into the fixed 50-byte char array of the struct.
// 1) fill the whole array with zeros
// 2) copy the text as UTF-8, but at most (size - 1) bytes
// So the last byte is always '\0', which marks the end of the text.
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

// Called from QML when Apply is clicked in Add mode.
// Builds an AddDataCmd, turns it into raw bytes and emits sendBytes.
void Controller::sendAdd(unsigned int uniqueId, double lat, double longi, const QString &comment)
{
    Protocol::AddDataCmd command{};          // {} = all fields start at zero
    command.cmdID = Protocol::CMD_ADD;
    command.UniqueID = uniqueId;
    command.lat = static_cast<float>(lat);   // QML gives double, struct wants float
    command.longi = static_cast<float>(longi);
    packComment(command.comment, sizeof(command.comment), comment);

    // The struct is packed (no padding), so its memory is exactly the bytes
    // the server expects (63 bytes).
    QByteArray data(reinterpret_cast<const char *>(&command), sizeof(command));
    emit sendBytes(data);   // goes to TcpSender::sendCommand in the sender thread
}

// Called from QML when Apply is clicked in Update mode.
// Same as sendAdd, only cmdID differs (UpdateDataCmd is the same struct).
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

// Called from QML when Delete is clicked. Only the ID is needed (5 bytes).
void Controller::sendDelete(unsigned int uniqueId)
{
    Protocol::DeleteDataCmd command{};
    command.cmdID = Protocol::CMD_DELETE;
    command.UniqueID = uniqueId;

    QByteArray data(reinterpret_cast<const char *>(&command), sizeof(command));
    emit sendBytes(data);
}

// Server answered an Add. Only if Ack is success is the item put in the list.
// addOrReplace is used so the same function works for add and update.
// If Ack is fail, nothing happens (as the assignment says).
void Controller::onAddResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment)
{
    if (ack == Protocol::ACK_SUCCESS)
        m_model->addOrReplace(uniqueId, lat, longi, comment);
}

// Server answered an Update. On success the existing row is replaced.
void Controller::onUpdateResponse(unsigned char ack, unsigned int uniqueId, float lat, float longi, QString comment)
{
    if (ack == Protocol::ACK_SUCCESS)
        m_model->addOrReplace(uniqueId, lat, longi, comment);
}

// Server answered a Delete. On success the row is removed from the list.
void Controller::onDeleteResponse(unsigned char ack, unsigned int uniqueId)
{
    if (ack == Protocol::ACK_SUCCESS)
        m_model->removeById(uniqueId);
}
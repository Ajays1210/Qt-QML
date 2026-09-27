#include <QCoreApplication>
#include <QDebug>
#include <QMetaType>

#include "server.h"
#include "protocol.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qRegisterMetaType<Protocol::AddDataResp>("Protocol::AddDataResp");
    qRegisterMetaType<Protocol::UpdateDataResp>("Protocol::UpdateDataResp");
    qRegisterMetaType<Protocol::DeleteDataResp>("Protocol::DeleteDataResp");

    qInfo() << "======================================";
    qInfo() << "        QT TASK TCP/UDP SERVER";
    qInfo() << "======================================";

    Server server;
    server.start();

    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &server, &Server::stop);

    return app.exec();
}
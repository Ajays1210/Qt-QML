#include <QApplication>
#include "mainwindow.h"
#include "protocol.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qRegisterMetaType<Protocol::AddDataResp>("Protocol::AddDataResp");
    qRegisterMetaType<Protocol::UpdateDataResp>("Protocol::UpdateDataResp");
    qRegisterMetaType<Protocol::DeleteDataResp>("Protocol::DeleteDataResp");

    MainWindow window;
    window.show();

    return app.exec();
}
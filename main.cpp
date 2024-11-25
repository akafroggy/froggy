#include "tcpfiletransfer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TcpFileTransfer window;
    window.show();

    return app.exec();
}
